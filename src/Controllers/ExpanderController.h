#pragma once

#include <string>
#include "Models/TerminalCommand.h"
#include "Interfaces/IUartService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IUtilityService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Shells/HelpShell.h"
#include "States/GlobalState.h"

class ExpanderController {
public:
    // Constructor
    ExpanderController(ITerminalView& terminalView,
                 IInput& terminalInput,
                 IUtilityService& utilityService,
                 IUartService& uartService,
                 ArgTransformer& argTransformer,
                 UserInputManager& userInputManager,
                 HelpShell& helpShell,
                 IUartService* usbCdc = nullptr);  // optional USB-A host transport (Tab5)
    
    // Entry point for Expander cmds
    void handleCommand(const TerminalCommand& cmd);

    // Ensure Expander configured before use
    void ensureConfigured();

private:
    // Configure the Expander
    void handleConfig();

    // Handle the UART bridge with the Expander
    void handleBridge();

    // Send a command over UART and wait until the expected token appears in the
    // reply (or the timeout elapses). Used to auto-detect which expander is wired.
    bool probeExpander(const std::string& command, const std::string& expectedToken, uint32_t timeoutMs);

    ITerminalView& terminalView;
    IInput& terminalInput;
    IUtilityService& utilityService;
    IUartService& uartService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    IUartService* usbCdc = nullptr;       // USB-A host transport, if provided
    IUartService* activeUart = nullptr;   // selected transport (set in the ctor)
    GlobalState& state = GlobalState::getInstance();

    // Fixed UART settings for the Expander
    const uint32_t baud = 115200;
    const uint8_t dataBits = 8;
    const char parityChar = 'N';
    const uint8_t stopBits = 1;
    const bool inverted = false;

    bool configured = false;
};
