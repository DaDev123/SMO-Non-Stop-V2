#include "NonStop/NonStop-patches.h"
#include <exlaunch.hpp>

using namespace exl::patch;
using namespace exl::armv8;

void installSDFileRedirection();
void allowSaveOnSDCard();
void NonStopHooks();

namespace NonStopPatches {

    void SkipCutscenes() {
    // Auto-Skip
    CodePatcher{0x4EFAE8}.WriteInst(inst::Movz(reg::W0, 1));
    CodePatcher{0x5516A8}.WriteInst(inst::Movz(reg::W0, 0));
    CodePatcher{0x5516AC}.WriteInst(inst::Ret());
    
    // Skip the hack first demo
    CodePatcher{0xD2D48}.WriteInst(inst::Ret());
    CodePatcher{0xD2D30}.WriteInst(inst::Movz(reg::W0, 0));
    CodePatcher{0xD2D34}.WriteInst(inst::Ret());
    CodePatcher{0xD2F70}.WriteInst(inst::Movz(reg::W0, 1));
    CodePatcher{0xD2F74}.WriteInst(inst::Ret());
    CodePatcher{0xD2FC8}.WriteInst(inst::Ret());
    CodePatcher{0xD31FC}.WriteInst(inst::Ret());
    CodePatcher{0xD2FB4}.WriteInst(inst::Movz(reg::W0, 1));
    CodePatcher{0xD2FB8}.WriteInst(inst::Ret());


    }

    void SkipShineDemos() {
        // ShineMove cutscene skip
        CodePatcher{0x1CE2B4}.WriteInst(inst::Nop());
        CodePatcher{0x1CE1C8}.WriteInst(inst::Nop());
        
        // Shine (Moons) demo skip
        const uintptr_t shineDemoAddresses[] = {
            0x1D18F0, 0x1D1934, 0x4DBC20, 0x4DBF08, 0x4DBF14, 0x4DC1D4,
            0x4DEA48, 0x43F83C
        };
        for (auto addr : shineDemoAddresses) {
            CodePatcher{addr}.WriteInst(inst::Nop());
        }

        // Force demo completion flags
        const uintptr_t demoFlagAddresses[] = {
            0x4DBF54, 0x4DBF60, 0x4DC578, 0x4DC584, 0x4DC66C, 0x4DC8B0,
            0x4DCA64, 0x4DCA9C, 0x4DE2F8, 0x4DE6AC, 0x4DE710, 0x4DE974,
            0x4DE984, 0x4DE9F8, 0x4DEA70, 0x4DEB30, 0x4DED44, 0x4DED80,
            0x4DED90, 0x4DC740
        };
        for (auto addr : demoFlagAddresses) {
            CodePatcher{addr}.WriteInst(inst::Movz(reg::W0, 1));
        }
    }

    void SkipShineChipCutscenes() {
        // Skip startShineChipCompleteAnim call at 0x4d05fc
        CodePatcher{0x4d0600}.WriteInst(inst::Nop());
        // Skip updateCounterParts call at 0x4d0604
        CodePatcher{0x4d0608}.WriteInst(inst::Nop());
        // Skip isEndShineChipCompleteAnim check at 0x4d0624
        CodePatcher{0x4d0630}.WriteInst(inst::Nop());
        // Skip endShineChipCompleteAnim call at 0x4d066c
        CodePatcher{0x4d0670}.WriteInst(inst::Nop());
        
        CodePatcher{0x1CDB2C}.WriteInst(inst::Nop());
        CodePatcher{0x1D56DC}.WriteInst(inst::Movz(reg::W0, 1));
    }

    void SkipShineGrandDemos() {
        // Multi Moon demo skip
        CodePatcher{0x550168}.WriteInst(inst::Movz(reg::W0, 1));
        CodePatcher{0x55016C}.WriteInst(inst::Ret());
    }

    void SkipShineHealingDemos() {
        const uintptr_t healingAddresses[] = {
            0x4DF0AC, 0x4DF16C, 0x4EECB0, 0x4EED90, 0x4EED9C, 0x4DF180
        };
        for (auto addr : healingAddresses) {
            CodePatcher{addr}.WriteInst(inst::Movz(reg::W0, 1));
        }
    }

    void SkipKingdomSpecificCutscenes() {
        // Skip moving platform spawn on first moon
        CodePatcher{0x304694}.WriteInst(inst::Nop());
        
        // Skip pyramid door opening on second moon
        CodePatcher{0x2fbb90}.WriteInst(inst::Nop());
        CodePatcher{0x2fcf58}.WriteInst(inst::Nop());
    }

    void SkipOpeningDemo() {
        CodePatcher{0x4CA1E4}.WriteInst(inst::Movz(reg::W0, 0));
    }

    void RemoveCappyDialogs() {
        CodePatcher{0x0D12F0}.WriteInst(inst::Nop());
        CodePatcher{0x0D1350}.WriteInst(inst::Nop());
        CodePatcher{0x54BF1C}.WriteInst(inst::Nop());
    }

    void QualityOfLifePatches() {
        CodePatcher{0x45C69C}.WriteInst(inst::Nop()); // Remove assist mode ledge grabs
        CodePatcher{0x50FF48}.WriteInst(inst::Nop()); // Skip Odyssey travel cutscenes
        CodePatcher{0x311B00}.WriteInst(inst::Nop());
        CodePatcher{0x311B10}.WriteInst(inst::Nop());
        CodePatcher{0x4DB934}.WriteInst(inst::Nop()); // Skip Life Up Heart demos
        CodePatcher{0x2D250C}.WriteInst(inst::Nop()); // Skip Notes demos
    }

    void MiscPatches() {
        const uintptr_t miscAddresses[] = {
            0x4DEE70, 0x4DEE58, 0x4EEF14, 0x4EEF48, 0x4EEF08
        };
        for (auto addr : miscAddresses) {
            CodePatcher{addr}.WriteInst(inst::Movz(reg::W0, 1));
        }
    }

    void installNonStopPatches() {
        SkipCutscenes();
        SkipShineDemos();
        SkipShineChipCutscenes();
        SkipShineGrandDemos();
        SkipShineHealingDemos();
        SkipKingdomSpecificCutscenes();
        SkipOpeningDemo();
        RemoveCappyDialogs();
        QualityOfLifePatches();
        MiscPatches();
        
        NonStopHooks();
        //installSDFileRedirection();
        //allowSaveOnSDCard();
    }
}