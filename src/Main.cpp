#include <Util/Log.hpp>
#include <DotNyet/VM/VirtualMachine.hpp>
#include <DotNyet/Core/Exceptions.hpp>

int main() {
    Util::Logger logger("Main");
    logger.Info(".NYET v1.0");

    try {
        using namespace DotNyet;
        VM::VirtualMachine vm;
        vm.AddInstruction(std::make_unique<VM::PushInstruction>(Types::Value("åäö")));
        vm.AddInstruction(std::make_unique<VM::PrintInstruction>());
        vm.Run();
    } catch (const std::exception& e) {
        logger.Error("Exception caught [{}]: {}", typeid(e).name(), e.what());
        return 1;
    }

    return 0;
}
