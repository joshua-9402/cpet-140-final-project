#include "testUI.h"
#include <string>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>

#include "hello_imgui/hello_imgui.h"
#include "../../handler/db.h"
#include "../../handler/system.h"
#include "../../security/cryptography.h"

void testUI::displayTest() {
    ImGui::Text("THIS IS ONLY FOR TESTING/DEBUGGING PURPOSE ONLY");

    static char passkey[128] = "";
    static std::string hashedPasskey;
    static bool passkeyHashed = false;

    ImGui::Text("Passkey");
    ImGui::InputText("##passkey", passkey, IM_ARRAYSIZE(passkey));

    if (ImGui::Button("Hash Passkey")) {
        const std::string passkeyStr(passkey);
        if (!passkeyStr.empty()) {
            // Use 32 bytes (256 bits) for hash length - standard SHA-256 equivalent
            hashedPasskey = cryptography::hashKey(passkey, 32);
            passkeyHashed = true;
            std::cout << "Button clicked! Passkey: '" << passkeyStr << "'" << std::endl;
            std::cout << "Hashed result: '" << hashedPasskey << "'" << std::endl;
            std::cout << "Hash length: " << hashedPasskey.length() << std::endl;
        } else {
            passkeyHashed = false;
            hashedPasskey.clear();
            std::cout << "Button clicked but passkey is empty!" << std::endl;
        }
    }

    ImGui::Spacing();

    // Always show if the flag is set, even if hash is empty (for debugging)
    if (passkeyHashed) {
        ImGui::Text("Passkey was hashed!");
        ImGui::Spacing();

        if (!hashedPasskey.empty()) {
            const float height = ImGui::GetTextLineHeightWithSpacing() * 3;
            ImGui::BeginChild("PasskeyDisplayPanel", ImVec2(0.0f, height), true);
            ImGui::TextWrapped("Hashed Passkey:");
            ImGui::TextWrapped("%s", hashedPasskey.c_str());
            ImGui::EndChild();
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hash result is empty! Check console output.");
        }
    }

    static int keySizeBits = 128; // Default key size
    ImGui::InputInt("Key Size (bits)", &keySizeBits, 8, 64);

    static bool keyGenerated = false;
    static std::vector<unsigned char> storedKeyRaw;
    static std::string storedKeyHashed;
    static std::string storedKeyHashedSalted;
    static int storedKeySizeBits = 0;

    if (ImGui::Button("Generate Key")) {
        storedKeyRaw = cryptography::generateKey(keySizeBits);
        storedKeyHashed = cryptography::toHex(storedKeyRaw);
        storedKeyHashedSalted = cryptography::saltKey(storedKeyHashed);
        storedKeySizeBits = keySizeBits;
        keyGenerated = true;
    }

    if (keyGenerated) {
        ImGui::Spacing();
        const float wrapWidth = ImGui::GetContentRegionAvail().x;
        const float height = ImGui::GetTextLineHeightWithSpacing() +
                             ImGui::CalcTextSize(storedKeyHashed.c_str(), nullptr, false, wrapWidth).y +
                             ImGui::CalcTextSize(storedKeyHashedSalted.c_str(), nullptr, false, wrapWidth).y;
        ImGui::BeginChild("KeyDisplayPanel", ImVec2(0, height), true);
        ImGui::Text("The key size in bits is: %d", storedKeySizeBits);
        ImGui::TextWrapped("The key in hex: %s", storedKeyHashed.c_str());
        ImGui::TextWrapped("The key in hex and salted: %s", storedKeyHashedSalted.c_str());
        ImGui::EndChild();
    }

    // Test encryption and decryption
    static const std::string location = "test.txt";
    static std::string encryptionStatus;
    static std::string decryptionStatus;

    ImGui::Spacing();
    ImGui::Text("Encryption/Decryption Test");

    if (ImGui::Button("Encrypt")) {
        // Ensure we have a key; generate with the user-selected size if missing
        if (!keyGenerated || storedKeyRaw.empty()) {
            storedKeyRaw = cryptography::generateKey(keySizeBits);
            storedKeyHashed = cryptography::toHex(storedKeyRaw);
            storedKeyHashedSalted = cryptography::saltKey(storedKeyHashed);
            storedKeySizeBits = keySizeBits;
            keyGenerated = true;
        }

        if (storedKeyRaw.empty()) {
            encryptionStatus = "ERROR: Key generation failed. Cannot encrypt.";
        } else {
            if (cryptography::encryptFile(location, storedKeyRaw)) {
                encryptionStatus = "SUCCESS: File encrypted to " + location + ".enc";
                system::deleteFile(location); // Delete the unencrypted file after successful encryption
            } else {
                encryptionStatus = "ERROR: Encryption failed. Check if file exists and key is correct.";
            }
        }
    }

    if (ImGui::Button("Decrypt")) {
        if (!keyGenerated || storedKeyRaw.empty()) {
            decryptionStatus = "ERROR: No key available. Generate or load a key before decrypting.";
        } else {
            if (cryptography::decryptFile(location + ".enc", storedKeyRaw)) {
                decryptionStatus = "SUCCESS: File decrypted (replaced " + location + ".enc with decrypted content)";
                system::deleteFile(location + ".enc"); // Delete the encrypted file after successful decryption
            } else {
                decryptionStatus = "ERROR: Decryption failed. Check if encrypted file exists and key is correct.";
            }
        }
    }

    if (!encryptionStatus.empty()) {
        ImGui::Spacing();
        if (encryptionStatus.find("SUCCESS") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", encryptionStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", encryptionStatus.c_str());
        }
    }

    if (!decryptionStatus.empty()) {
        ImGui::Spacing();
        if (decryptionStatus.find("SUCCESS") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", decryptionStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", decryptionStatus.c_str());
        }
    }
}