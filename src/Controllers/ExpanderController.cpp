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
                           HelpShell& helpShell)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      utilityService(utilityService),
      uartService(uartService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      helpShell(helpShell) {
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
        uartService.write("\n");
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
        while (uartService.available()) {
            char c = uartService.read();
            terminalView.print(std::string(1, c));
        }

        // ESC seq
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            if (c == '\x1B') {
                uartService.write(c);

                uint32_t start = utilityService.nowMs();
                while (utilityService.nowMs() - start < 20) {
                    char c2 = terminalInput.readChar();
                    if (c2 != KEY_NONE) {
                        uartService.write(c2);

                        start = utilityService.nowMs();
                        while (utilityService.nowMs() - start < 20) {
                            char c3 = terminalInput.readChar();
                            if (c3 != KEY_NONE) {
                                uartService.write(c3);
                                break;
                            }
                        }
                        break;
                    }
                }
                continue;
            }

            uartService.write(c);

            if (c == '\r' || c == '\n') {
                if (txLine == "exit") {
                    terminalView.println("\n\n\rExpander session closed.");
                    terminalView.println("Returning to ESP32 Bit Pirate...\n");
                    uartService.flush();
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

    uint32_t config = uartService.buildUartConfig(dataBits, parityChar, stopBits);
    uartService.configure(baud, config, rxPin, txPin, inverted);

    terminalView.println("Expander UART configured (115200 8N1).");
    terminalView.println("Sending handshake...");

    // Flush RX
    while (uartService.available()) {
        uartService.read();
    }

    utilityService.sleepMs(100);

    // Send a few ENTER to bring the slave back to its main prompt
    for (int i = 0; i < 8; ++i) {
        uartService.write('\r');
        uartService.write('\n');
        utilityService.sleepMs(20);
    }

    // flush what came back after the reset
    while (uartService.available()) {
        uartService.read();
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
    while (uartService.available()) {
        uartService.read();
    }

    uartService.write(command);

    std::string rxBuffer;
    uint32_t start = utilityService.nowMs();

    while (utilityService.nowMs() - start < timeoutMs) {
        while (uartService.available()) {
            char c = uartService.read();
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
