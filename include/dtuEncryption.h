#ifndef DTUENCRYPTION_H
#define DTUENCRYPTION_H

#include <Arduino.h>
#include <cstring>
#include <mbedtls/aes.h>
#include <mbedtls/gcm.h>
#include <mbedtls/sha256.h>

/**
 * @brief DTU Encryption Manager for AES-128-GCM encrypted communication
 *
 * Handles encryption/decryption for DTU Wlite S and other encrypted DTU variants.
 * Protocol: AES-128-GCM with SHA256-derived keys and nonces
 *
 * Supports:
 * - Auto-detection of encrypted DTU via app_information_data.dtu_info.dfs
 * - Transparent encryption of commands
 * - Transparent decryption of responses
 * - Separate handling of NOT_ENCRYPTED_COMMANDS (read-only operations)
 * - Dynamic enc_rand updates from DTU responses
 */
class DTUEncryption
{
public:
    // List of command tags that should NOT be encrypted (read-only operations)
    static constexpr uint16_t NOT_ENCRYPTED_COMMANDS[] = {0x0001, 0x0002}; // APP_INFO_DATA_REQ, HEARTBEAT
    static constexpr size_t NOT_ENCRYPTED_COMMANDS_SIZE = 2;

    /**
     * @brief Initialize encryption manager
     */
    DTUEncryption();

    /**
     * @brief Destructor - cleanup mbedtls context
     */
    ~DTUEncryption();

    /**
     * @brief Check if encryption is enabled for this DTU
     * @return true if DTU is using encrypted communication
     */
    boolean isEncrypted() const { return m_isEncrypted; }

    /**
     * @brief Enable encryption with provided enc_rand
     * @param encRand Pointer to enc_rand bytes (16 bytes)
     * @param encRandLen Length of enc_rand (should be 16)
     * @return true if encryption was successfully enabled
     */
    boolean enableEncryption(const uint8_t *encRand, size_t encRandLen);

    /**
     * @brief Disable encryption (fallback mode)
     */
    void disableEncryption() { m_isEncrypted = false; }

    /**
     * @brief Update enc_rand (called when DTU returns new random in response)
     * @param encRand Pointer to new enc_rand bytes (16 bytes)
     * @param encRandLen Length of enc_rand
     * @return true if update was successful
     */
    boolean updateEncRand(const uint8_t *encRand, size_t encRandLen);

    /**
     * @brief Check if a command should be encrypted
     * @param commandTag Command tag (from protobuf message)
     * @return true if command should be encrypted, false if it's a read-only command
     */
    boolean shouldEncryptCommand(uint16_t commandTag) const;

    /**
     * @brief Encrypt a command payload using AES-128-GCM
     *
     * Input plaintext format: [protobuf_data + 16_byte_auth_tag]
     * Output ciphertext format: [ciphertext_without_auth_tag]
     *
     * @param commandTag Command identifier (used for AAD)
     * @param sequence Sequence number (used for nonce derivation)
     * @param plaintextData Input plaintext with embedded 16-byte auth tag
     * @param plaintextLen Length of plaintext (includes 16-byte tag)
     * @param ciphertextData Output buffer for ciphertext (will be plaintextLen - 16 bytes)
     * @param ciphertextMaxLen Maximum size of ciphertext buffer
     * @param outCiphertextLen Output: actual ciphertext length
     * @return true if encryption successful, false otherwise
     */
    boolean encryptCommand(
        uint16_t commandTag,
        uint16_t sequence,
        const uint8_t *plaintextData,
        size_t plaintextLen,
        uint8_t *ciphertextData,
        size_t ciphertextMaxLen,
        size_t &outCiphertextLen);

    /**
     * @brief Decrypt a response payload using AES-128-GCM
     *
     * Input ciphertext format: [ciphertext_only]
     * Output plaintext format: [protobuf_data + 16_byte_auth_tag]
     *
     * @param commandTag Command identifier (used for AAD)
     * @param sequence Sequence number (used for nonce derivation)
     * @param ciphertextData Input ciphertext
     * @param ciphertextLen Length of ciphertext
     * @param plaintextData Output buffer for plaintext (will include 16-byte auth tag)
     * @param plaintextMaxLen Maximum size of plaintext buffer
     * @param outPlaintextLen Output: actual plaintext length (includes 16-byte tag)
     * @return true if decryption successful, false otherwise
     */
    boolean decryptResponse(
        uint16_t commandTag,
        uint16_t sequence,
        const uint8_t *ciphertextData,
        size_t ciphertextLen,
        uint8_t *plaintextData,
        size_t plaintextMaxLen,
        size_t &outPlaintextLen);

    /**
     * @brief Get current enc_rand for debugging/logging
     * @param output Buffer to receive enc_rand hex string
     * @param outputLen Max length of output buffer
     */
    void getEncRandHex(char *output, size_t outputLen) const;

    /**
     * @brief Reset encryption state (on connection reset)
     */
    void reset();

private:
    boolean m_isEncrypted = false;
    uint8_t m_encRand[16] = {0};

    // mbedtls contexts
    mbedtls_gcm_context m_gcm_ctx;
    mbedtls_aes_context m_aes_ctx;

    /**
     * @brief Derive AES-128 key from enc_rand using SHA256
     * @param encRand Input enc_rand (16 bytes)
     * @param keyOutput Output key buffer (16 bytes)
     */
    static void deriveAes128Key(const uint8_t *encRand, uint8_t *keyOutput);

    /**
     * @brief Derive nonce for AES-GCM using triple SHA256
     *
     * nonce = SHA256(SHA256(SHA256(tag_bytes + seq_bytes + encRand)))[:12]
     *
     * @param encRand Input enc_rand (16 bytes)
     * @param u16Tag Command tag (converted to bytes)
     * @param u16Seq Sequence number (converted to bytes)
     * @param nonceOutput Output nonce buffer (12 bytes)
     */
    static void deriveNonce(
        const uint8_t *encRand,
        uint16_t u16Tag,
        uint16_t u16Seq,
        uint8_t *nonceOutput);

    /**
     * @brief Helper: SHA256 hash
     * @param input Input data
     * @param inputLen Input length
     * @param output Output hash (32 bytes)
     */
    static void sha256Hash(const uint8_t *input, size_t inputLen, uint8_t *output);
};

#endif // DTUENCRYPTION_H
