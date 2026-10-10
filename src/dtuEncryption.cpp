#include "dtuEncryption.h"

/**
 * DTU Encryption Implementation
 * AES-128-GCM with SHA256-derived keys and nonces
 * Protocol compatible with hoymiles-wifi v0.5.0+
 */

DTUEncryption::DTUEncryption()
{
    mbedtls_gcm_init(&m_gcm_ctx);
    mbedtls_aes_init(&m_aes_ctx);
    m_dtuTimestampOffset = 0;
}

DTUEncryption::~DTUEncryption()
{
    mbedtls_gcm_free(&m_gcm_ctx);
    mbedtls_aes_free(&m_aes_ctx);
}

boolean DTUEncryption::enableEncryption(const uint8_t *encRand, size_t encRandLen)
{
    if (encRand == nullptr || encRandLen != 16)
    {
        Serial.println(F("DTUEncryption: Invalid enc_rand size"));
        return false;
    }

    // Store enc_rand
    memcpy(m_encRand, encRand, 16);
    m_isEncrypted = true;

    Serial.println(F("DTUEncryption: Encryption enabled"));
    getEncRandHex(nullptr, 0); // Log enc_rand

    return true;
}

boolean DTUEncryption::updateEncRand(const uint8_t *encRand, size_t encRandLen)
{
    if (encRand == nullptr || encRandLen != 16)
    {
        Serial.println(F("DTUEncryption: Invalid enc_rand update size"));
        return false;
    }

    memcpy(m_encRand, encRand, 16);
    Serial.println(F("DTUEncryption: enc_rand updated"));

    return true;
}

boolean DTUEncryption::shouldEncryptCommand(uint16_t commandTag) const
{
    if (!m_isEncrypted)
        return false;

    // Check if command is in NOT_ENCRYPTED list
    for (size_t i = 0; i < NOT_ENCRYPTED_COMMANDS_SIZE; i++)
    {
        if (commandTag == NOT_ENCRYPTED_COMMANDS[i])
        {
            return false;
        }
    }

    return true;
}

void DTUEncryption::sha256Hash(const uint8_t *input, size_t inputLen, uint8_t *output)
{
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0); // 0 = SHA256, 1 = SHA224
    mbedtls_sha256_update(&sha256_ctx, input, inputLen);
    mbedtls_sha256_finish(&sha256_ctx, output);
    mbedtls_sha256_free(&sha256_ctx);
}

void DTUEncryption::deriveAes128Key(const uint8_t *encRand, uint8_t *keyOutput)
{
    // Key = SHA256(enc_rand)
    sha256Hash(encRand, 16, keyOutput);
    // Output is 32 bytes, we use first 16 for AES-128
}

void DTUEncryption::deriveNonce(
    const uint8_t *encRand,
    uint16_t u16Tag,
    uint16_t u16Seq,
    uint8_t *nonceOutput)
{
    // Build buffer: tag_bytes (2) + seq_bytes (2) + enc_rand (16) = 20 bytes
    uint8_t buffer[20];
    buffer[0] = (u16Tag >> 8) & 0xFF;
    buffer[1] = u16Tag & 0xFF;
    buffer[2] = (u16Seq >> 8) & 0xFF;
    buffer[3] = u16Seq & 0xFF;
    memcpy(buffer + 4, encRand, 16);

    // Triple SHA256: SHA256(SHA256(SHA256(buffer)))
    uint8_t hash1[32], hash2[32], hash3[32];
    sha256Hash(buffer, 20, hash1);
    sha256Hash(hash1, 32, hash2);
    sha256Hash(hash2, 32, hash3);

    // Use first 12 bytes of final hash as nonce
    memcpy(nonceOutput, hash3, 12);
}

boolean DTUEncryption::encryptCommand(
    uint16_t commandTag,
    uint16_t sequence,
    const uint8_t *plaintextData,
    size_t plaintextLen,
    uint8_t *ciphertextData,
    size_t ciphertextMaxLen,
    size_t &outCiphertextLen)
{
    if (!m_isEncrypted)
    {
        Serial.println(F("DTUEncryption: Encryption not enabled"));
        return false;
    }

    if (plaintextLen < 16)
    {
        Serial.println(F("DTUEncryption: Plaintext too short (must include 16-byte tag)"));
        return false;
    }

    // Enhanced logging for troubleshooting
    Serial.println("DTUEncryption: Encrypting command tag: 0x" + String(commandTag, HEX) + 
                  ", seq: " + String(sequence) + 
                  ", plaintextLen: " + String(plaintextLen));

    // Ciphertext length = plaintext length - 16 (auth tag is computed, not included in output)
    size_t expectedCiphertextLen = plaintextLen - 16;

    if (ciphertextMaxLen < expectedCiphertextLen)
    {
        Serial.printf("DTUEncryption: Ciphertext buffer too small (%u < %u)\n", ciphertextMaxLen, expectedCiphertextLen);
        return false;
    }

    // Derive key and nonce
    uint8_t key[16];
    uint8_t nonce[12];
    deriveAes128Key(m_encRand, key);
    deriveNonce(m_encRand, commandTag, sequence, nonce);

    // AAD (Additional Authenticated Data) = tag + sequence
    uint8_t aad[4];
    aad[0] = (commandTag >> 8) & 0xFF;
    aad[1] = commandTag & 0xFF;
    aad[2] = (sequence >> 8) & 0xFF;
    aad[3] = sequence & 0xFF;

    // Setup GCM for encryption
    mbedtls_gcm_setkey(&m_gcm_ctx, MBEDTLS_CIPHER_ID_AES, key, 128);

    // Encrypt: plaintext (without tag) -> ciphertext
    // The plaintext data contains 16 bytes of pre-computed auth tag at the end
    // We encrypt the data portion only
    int result = mbedtls_gcm_crypt_and_tag(
        &m_gcm_ctx,
        MBEDTLS_GCM_ENCRYPT,
        plaintextLen - 16,                       // Length to encrypt (excluding the tag)
        nonce,                                   // Nonce
        12,                                      // Nonce length
        aad,                                     // Additional authenticated data
        4,                                       // AAD length
        plaintextData,                           // Input plaintext
        ciphertextData,                          // Output ciphertext
        16,                                      // Tag length
        ciphertextData + expectedCiphertextLen); // Output tag location (appended to ciphertext)

    if (result != 0)
    {
        Serial.printf("DTUEncryption: AES-GCM encryption failed: %d\n", result);
        return false;
    }

    outCiphertextLen = expectedCiphertextLen + 16; // Include the tag
    Serial.println("DTUEncryption: Encryption successful, ciphertextLen: " + String(outCiphertextLen));

    return true;
}

boolean DTUEncryption::decryptResponse(
    uint16_t commandTag,
    uint16_t sequence,
    const uint8_t *ciphertextData,
    size_t ciphertextLen,
    uint8_t *plaintextData,
    size_t plaintextMaxLen,
    size_t &outPlaintextLen)
{
    if (!m_isEncrypted)
    {
        Serial.println(F("DTUEncryption: Encryption not enabled"));
        return false;
    }

    if (ciphertextLen < 16)
    {
        Serial.println(F("DTUEncryption: Ciphertext too short (must include 16-byte tag)"));
        return false;
    }

    // Enhanced logging for troubleshooting
    Serial.println("DTUEncryption: Decrypting response tag: 0x" + String(commandTag, HEX) + 
                  ", seq: " + String(sequence) + 
                  ", ciphertextLen: " + String(ciphertextLen));

    // Plaintext length = ciphertext length (tag is verified but included in plaintext output)
    size_t expectedPlaintextLen = ciphertextLen;

    if (plaintextMaxLen < expectedPlaintextLen)
    {
        Serial.printf("DTUEncryption: Plaintext buffer too small (%u < %u)\n", plaintextMaxLen, expectedPlaintextLen);
        return false;
    }

    // Derive key and nonce
    uint8_t key[16];
    uint8_t nonce[12];
    deriveAes128Key(m_encRand, key);
    deriveNonce(m_encRand, commandTag, sequence, nonce);

    // AAD (Additional Authenticated Data) = tag + sequence
    uint8_t aad[4];
    aad[0] = (commandTag >> 8) & 0xFF;
    aad[1] = commandTag & 0xFF;
    aad[2] = (sequence >> 8) & 0xFF;
    aad[3] = sequence & 0xFF;

    // Setup GCM for decryption
    mbedtls_gcm_setkey(&m_gcm_ctx, MBEDTLS_CIPHER_ID_AES, key, 128);

    // Extract tag from end of ciphertext
    const uint8_t *authTag = ciphertextData + ciphertextLen - 16;
    size_t dataLen = ciphertextLen - 16;

    // Decrypt and verify
    int result = mbedtls_gcm_auth_decrypt(
        &m_gcm_ctx,
        dataLen,        // Data length (excluding tag)
        nonce,          // Nonce
        12,             // Nonce length
        aad,            // Additional authenticated data
        4,              // AAD length
        authTag,        // Expected authentication tag
        16,             // Tag length
        ciphertextData, // Input ciphertext
        plaintextData); // Output plaintext

    if (result != 0)
    {
        Serial.printf("DTUEncryption: [ENCRYPTION ERROR] AES-GCM decryption/authentication failed: %d\n", result);
        return false;
    }

    // Copy tag to output plaintext (for protobuf parsing)
    memcpy(plaintextData + dataLen, authTag, 16);
    outPlaintextLen = expectedPlaintextLen;

    Serial.println("DTUEncryption: [ENCRYPTION] Successfully decrypted response (tag: 0x" + 
                  String(commandTag, HEX) + ", seq: " + String(sequence) + 
                  ", len: " + String(outPlaintextLen) + " bytes)");

    return true;
}

void DTUEncryption::getEncRandHex(char *output, size_t outputLen) const
{
    char hexBuffer[33]; // 32 hex chars + null
    for (int i = 0; i < 16; i++)
    {
        snprintf(hexBuffer + i * 2, 3, "%02x", m_encRand[i]);
    }

    Serial.printf("DTUEncryption: enc_rand = %s\n", hexBuffer);

    if (output != nullptr && outputLen > 0)
    {
        strncpy(output, hexBuffer, outputLen - 1);
        output[outputLen - 1] = '\0';
    }
}

void DTUEncryption::reset()
{
    m_isEncrypted = false;
    memset(m_encRand, 0, 16);
    m_dtuTimestampOffset = 0;
    Serial.println(F("DTUEncryption: Reset"));
}

// NEW: Timestamp synchronization methods
void DTUEncryption::setDtuTimestamp(uint32_t dtuTimestamp)
{
    time_t now = time(nullptr);
    m_dtuTimestampOffset = static_cast<int32_t>(dtuTimestamp) - static_cast<int32_t>(now);
    Serial.println("DTUEncryption: DTU clock offset: " + String(m_dtuTimestampOffset) + "s");
}

uint32_t DTUEncryption::getAdjustedTimestamp() const
{
    return static_cast<uint32_t>(time(nullptr)) + m_dtuTimestampOffset;
}
