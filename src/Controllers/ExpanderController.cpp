#include "Controllers/ExpanderController.h"

/*
Constructor
*/
ExpanderController::ExpanderController(ITerminalView& terminalView,
                           IInput& terminalInput,
                           IUtilityService& utilityService,
                           IUartService& uartService,
                           ArgTransformer& argTransformer,
                           UserInputManager& userInputManager,
                           HelpShell& helpShell,
                           IUartService* usbCdc)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      utilityService(utilityService),
      uartService(uartService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      helpShell(helpShell),
      usbCdc(usbCdc),
      activeUart(&uartService) {
}

/*
Entry point for Expander command
*/
void ExpanderController::handleCommand(const TerminalCommand& cmd) {
    if (!configured) {
        handleConfig();
    } else {
        handleBridge();
    }
}

/*
Ensure configured
*/
void ExpanderController::ensureConfigured() {
    if (!configured) {
        handleConfig();
    } else {
        activeUart->write("\n");
        handleBridge();
    }

}

/*
Bridge
*/
void ExpanderController::handleBridge() {
    terminalView.println("Expander Connected: Starting... Type 'exit' to stop.");

    std::string txLine;

    while (true) {
        while (activeUart->available()) {
            char c = activeUart->read();
            terminalView.print(std::string(1, c));
        }

        // ESC seq
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            if (c == '\x1B') {
                activeUart->write(c);

                uint32_t start = utilityService.nowMs();
                while (utilityService.nowMs() - start < 20) {
                    char c2 = terminalInput.readChar();
                    if (c2 != KEY_NONE) {
                        activeUart->write(c2);

                        start = utilityService.nowMs();
                        while (utilityService.nowMs() - start < 20) {
                            char c3 = terminalInput.readChar();
                            if (c3 != KEY_NONE) {
                                activeUart->write(c3);
                                break;
                            }
                        }
                        break;
                    }
                }
                continue;
            }

            activeUart->write(c);

            if (c == '\r' || c == '\n') {
                if (txLine == "exit") {
                    terminalView.println("\n\n\rExpander session closed.");
                    terminalView.println("Returning to ESP32 Bit Pirate...\n");
                    activeUart->flush();
                    configured = false;
                    return;
                }
                txLine.clear();
            } else if (c == '\b' || c == 127) {
                if (!txLine.empty()) {
                    txLine.pop_back();
                }
            } else {
                txLine += c;
            }
        }

        utilityService.sleepMs(1);
    }
}

/*
Config
*/
void ExpanderController::handleConfig() {
    // Transport choice: on the Tab5, offer the USB-A host path (a C5 whose USB-C
    // plugs into the Tab5's USB-A port). Otherwise use the GPIO UART.
    bool useUsb = false;
    if (usbCdc != nullptr) {
        useUsb = userInputManager.readYesNo("Use USB-A host for the C5 (its USB-C)?", true);
    }

    if (useUsb) {
        activeUart = usbCdc;
        terminalView.println("USB-A host: plug the C5's USB-C into the Tab5 USB-A port.");
        terminalView.println("Starting USB host...");
        activeUart->configure(baud, 0, 0, 0, false);  // brings up the USB host + opens the CDC device
        if (!activeUart->isInstalled()) {
            terminalView.println("No USB CDC device found on USB-A.");
            terminalView.println("Check the cable and that the C5 is powered.\n");
            activeUart = &uartService;
            configured = false;
            state.setCurrentMode(ModeEnum::HIZ);
            return;
        }
        terminalView.println("USB-A host ready.");
    } else {
        activeUart = &uartService;
        terminalView.println("Expander UART Configuration:");

        auto forbidden = state.getProtectedPins();

        uint8_t rxPin = userInputManager.readValidatedPinNumber(
            "RX GPIO number",
            state.getUartRxPin(),
            forbidden
        );
        state.setUartRxPin(rxPin);
        forbidden.push_back(rxPin);

        uint8_t txPin = userInputManager.readValidatedPinNumber(
            "TX GPIO number",
            state.getUartTxPin(),
            forbidden
        );
        state.setUartTxPin(txPin);
        forbidden.push_back(txPin);

        uint32_t config = activeUart->buildUartConfig(dataBits, parityChar, stopBits);
        activeUart->configure(baud, config, rxPin, txPin, inverted);

        terminalView.println("Expander UART configured (115200 8N1).");
    }

    terminalView.println("Sending handshake...");

    // Flush RX
    while (activeUart->available()) {
        activeUart->read();
    }

    utilityService.sleepMs(100);

    // Send a few ENTER to bring the slave back to its main prompt
    for (int i = 0; i < 8; ++i) {
        activeUart->write('\r');
        activeUart->write('\n');
        utilityService.sleepMs(20);
    }

    // flush what came back after the reset
    while (activeUart->available()) {
        activeUart->read();
    }

    // Auto-detect the expander:
    //  - the ESP32 Bus Expander (geo-tp) answers "handshake" -> [[BP-HANDSHAKE-OK]]
    //  - a projectZero / JanOS ESP32-C5 runs an esp_console REPL: "ping" -> "pong"
    bool busExpander = probeExpander("handshake\n", "[[BP-HANDSHAKE-OK]]", 2000);
    bool janosC5 = false;
    if (!busExpander) {
        janosC5 = probeExpander("ping\n", "pong", 1500);
    }

    if (!busExpander && !janosC5) {
        terminalView.println("Expander handshake failed.");
        terminalView.println("Try to swap RX/TX GPIOs.");
        terminalView.println("Ensure the Expander (or C5) is powered.\n");
        configured = false;
        state.setCurrentMode(ModeEnum::HIZ);
        return;
    }

    if (janosC5) {
        terminalView.println("projectZero (JanOS) ESP32-C5 detected.");
        terminalView.println("");
        terminalView.println(" [ℹ️  INFORMATION] ");
        terminalView.println(" Connected to the C5 console. Try:");
        terminalView.println("   help, scan_networks, start_sniffer");
        terminalView.println("   packet_monitor <ch>, start_pcap radio");
        terminalView.println("   start_deauth, start_handshake, stop\n");
    } else {
        terminalView.println("Expander handshake OK.");
        terminalView.println("");
        terminalView.println(" [ℹ️  INFORMATION] ");
        terminalView.println(" You are now connected to the Expander.");
        terminalView.println(" All commands are sent directly to it.\n");
    }

    configured = true;
    handleBridge();
}

/*
Probe: send a command and scan the UART reply for an expected token
*/
bool ExpanderController::probeExpander(const std::string& command, const std::string& expectedToken, uint32_t timeoutMs) {
    // Flush any stale RX first
    while (activeUart->available()) {
        activeUart->read();
    }

    activeUart->write(command);

    std::string rxBuffer;
    uint32_t start = utilityService.nowMs();

    while (utilityService.nowMs() - start < timeoutMs) {
        while (activeUart->available()) {
            char c = activeUart->read();
            rxBuffer += c;

            if (rxBuffer.size() > 256) {
                rxBuffer.erase(0, rxBuffer.size() - 256);
            }

            if (rxBuffer.find(expectedToken) != std::string::npos) {
                return true;
            }
        }

        utilityService.sleepMs(5);
    }

    return false;
}
