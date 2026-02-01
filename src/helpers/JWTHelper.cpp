#include "JWTHelper.h"
#include <ArduinoJson.h>
#include <SHA256.h>
#include <string.h>
#include <stdlib.h>
#include "ed_25519.h"
#include "mbedtls/base64.h"
#include "Utils.h"

// Base64 URL encoding table (without padding)
static const char base64url_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

bool JWTHelper::createAuthToken(
  const mesh::LocalIdentity& identity,
  const char* audience,
  unsigned long issuedAt,
  unsigned long expiresIn,
  char* token,
  size_t tokenSize,
  const char* owner,
  const char* client,
  const char* email
) {
  if (!audience || !token || tokenSize == 0) {
    return false;
  }
  
  // Use current time if not specified
  if (issuedAt == 0) {
    issuedAt = time(nullptr);
  }
  
  // Create header
  char header[256];
  size_t headerLen = createHeader(header, sizeof(header));
  if (headerLen == 0) {
    return false;
  }
  
  // Get public key as UPPERCASE HEX string
  char publicKeyHex[65];
  mesh::Utils::toHex(publicKeyHex, identity.pub_key, PUB_KEY_SIZE);
  for (int i = 0; publicKeyHex[i]; i++) {
    publicKeyHex[i] = toupper(publicKeyHex[i]);
  }
  
  // Create payload
  char payload[512];
  size_t payloadLen = createPayload(publicKeyHex, audience, issuedAt, expiresIn, payload, sizeof(payload), owner, client, email);
  if (payloadLen == 0) {
    return false;
  }
  
  // Create signing input: header.payload
  char signingInput[768];
  size_t signingInputLen = headerLen + 1 + payloadLen;
  if (signingInputLen >= sizeof(signingInput)) {
    return false;
  }
  
  memcpy(signingInput, header, headerLen);
  signingInput[headerLen] = '.';
  memcpy(signingInput + headerLen + 1, payload, payloadLen);
  
  // Sign the data using direct Ed25519 signing
  uint8_t signature[64];
  mesh::LocalIdentity identity_copy = identity;
  
  uint8_t export_buffer[96];
  size_t exported_size = identity_copy.writeTo(export_buffer, sizeof(export_buffer));
  
  if (exported_size != 96) {
    return false;
  }
  
  uint8_t* private_key = export_buffer;
  uint8_t* public_key = export_buffer + 64;
  
  ed25519_sign(signature, (const unsigned char*)signingInput, signingInputLen, public_key, private_key);
  
  // Verify the signature locally
  int verify_result = ed25519_verify(signature, (const unsigned char*)signingInput, signingInputLen, public_key);
  if (verify_result != 1) {
    Serial.println("JWTHelper: Signature verification failed!");
    return false;
  }
  
  // Convert signature to hex
  char signatureHex[129];
  for (int i = 0; i < 64; i++) {
    sprintf(signatureHex + (i * 2), "%02X", signature[i]);
  }
  signatureHex[128] = '\0';
  
  // Create final token: header.payload.signatureHex (MeshCore Decoder format)
  size_t sigHexLen = strlen(signatureHex);
  size_t totalLen = headerLen + 1 + payloadLen + 1 + sigHexLen;
  if (totalLen >= tokenSize) {
    return false;
  }
  
  memcpy(token, header, headerLen);
  token[headerLen] = '.';
  memcpy(token + headerLen + 1, payload, payloadLen);
  token[headerLen + 1 + payloadLen] = '.';
  memcpy(token + headerLen + 1 + payloadLen + 1, signatureHex, sigHexLen);
  token[totalLen] = '\0';

  return true;
}

size_t JWTHelper::base64UrlEncode(const uint8_t* input, size_t inputLen, char* output, size_t outputSize) {
  if (!input || !output || outputSize == 0) {
    return 0;
  }
  
  size_t outlen = 0;
  int ret = mbedtls_base64_encode((unsigned char*)output, outputSize - 1, &outlen, input, inputLen);
  
  if (ret != 0) {
    return 0;
  }
  
  // Convert to base64 URL format in-place (replace + with -, / with _, remove padding =)
  for (size_t i = 0; i < outlen; i++) {
    if (output[i] == '+') {
      output[i] = '-';
    } else if (output[i] == '/') {
      output[i] = '_';
    }
  }
  
  // Remove padding '=' characters
  while (outlen > 0 && output[outlen-1] == '=') {
    outlen--;
  }
  output[outlen] = '\0';
  return outlen;
}

size_t JWTHelper::createHeader(char* output, size_t outputSize) {
  // Create JWT header: {"alg":"Ed25519","typ":"JWT"}
  DynamicJsonDocument doc(256);
  doc["alg"] = "Ed25519";
  doc["typ"] = "JWT";
  
  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
  if (len == 0 || len >= sizeof(jsonBuffer)) {
    return 0;
  }
  
  return base64UrlEncode((uint8_t*)jsonBuffer, len, output, outputSize);
}

size_t JWTHelper::createPayload(
  const char* publicKey,
  const char* audience,
  unsigned long issuedAt,
  unsigned long expiresIn,
  char* output,
  size_t outputSize,
  const char* owner,
  const char* client,
  const char* email
) {
  // Create JWT payload
  DynamicJsonDocument doc(512);
  doc["publicKey"] = publicKey;
  doc["aud"] = audience;
  doc["iat"] = issuedAt;
  
  if (expiresIn > 0) {
    doc["exp"] = issuedAt + expiresIn;
  }
  
  // Add optional owner field if provided
  if (owner && strlen(owner) > 0) {
    doc["owner"] = owner;
  }
  
  // Add optional client field if provided
  if (client && strlen(client) > 0) {
    doc["client"] = client;
  }
  
  // Add optional email field if provided
  if (email && strlen(email) > 0) {
    doc["email"] = email;
  }
  
  char jsonBuffer[512];
  size_t len = serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
  if (len == 0 || len >= sizeof(jsonBuffer)) {
    return 0;
  }
  
  return base64UrlEncode((uint8_t*)jsonBuffer, len, output, outputSize);
}

size_t JWTHelper::base64UrlDecode(const char* input, uint8_t* output, size_t outputSize) {
  if (!input || !output || outputSize == 0) {
    return 0;
  }
  
  // Convert base64url to base64 (replace - with +, _ with /)
  size_t inputLen = strlen(input);
  if (inputLen == 0) {
    return 0;
  }
  
  // Use heap allocation for large buffer to avoid stack overflow in MQTT task
  char* base64Input = (char*)malloc(inputLen + 4 + 1); // +4 for padding, +1 for null terminator
  if (!base64Input) {
    Serial.printf("JWTHelper::base64UrlDecode: Failed to allocate buffer\n");
    return 0;
  }
  
  strncpy(base64Input, input, inputLen);
  base64Input[inputLen] = '\0';
  
  // Add padding if needed
  size_t padding = (4 - (inputLen % 4)) % 4;
  for (size_t i = 0; i < padding; i++) {
    base64Input[inputLen + i] = '=';
  }
  base64Input[inputLen + padding] = '\0';
  
  // Convert base64url to base64
  for (size_t i = 0; i < inputLen; i++) {
    if (base64Input[i] == '-') {
      base64Input[i] = '+';
    } else if (base64Input[i] == '_') {
      base64Input[i] = '/';
    }
  }
  
  size_t outlen = 0;
  int ret = mbedtls_base64_decode(output, outputSize, &outlen, (const unsigned char*)base64Input, strlen(base64Input));
  
  // Free the buffer before returning
  free(base64Input);
  
  if (ret != 0) {
    Serial.printf("JWTHelper::base64UrlDecode: mbedtls_base64_decode failed with code %d (inputLen=%u)\n", 
                  ret, inputLen);
    return 0;
  }
  
  return outlen;
}

bool JWTHelper::verifyToken(
  const char* token,
  const uint8_t* expected_public_key,
  size_t key_len,
  char* extracted_public_key,
  size_t extracted_key_size,
  char* extracted_nonce,
  size_t nonce_size,
  unsigned long* issued_at,
  unsigned long* expires_at
) {
  Serial.printf("JWTHelper::verifyToken: Starting\n");
  
  if (!token || !extracted_public_key || extracted_key_size < 65) {
    Serial.printf("JWTHelper::verifyToken: Invalid parameters\n");
    return false;
  }
  
  // Parse token: header.payload.signature
  const char* dot1 = strchr(token, '.');
  if (!dot1) {
    Serial.printf("JWTHelper::verifyToken: No first dot\n");
    return false;
  }
  
  const char* dot2 = strchr(dot1 + 1, '.');
  if (!dot2) {
    Serial.printf("JWTHelper::verifyToken: No second dot\n");
    return false;
  }
  
  size_t headerLen = dot1 - token;
  size_t payloadLen = dot2 - (dot1 + 1);
  size_t signatureLen = strlen(dot2 + 1);
  
  Serial.printf("JWTHelper::verifyToken: headerLen=%u, payloadLen=%u, signatureLen=%u\n", 
                headerLen, payloadLen, signatureLen);
  
  // Decode header - extract header part first (before first dot)
  // Use heap allocation to reduce stack usage
  char* header_b64 = (char*)malloc(headerLen + 1);
  if (!header_b64) {
    Serial.printf("JWTHelper::verifyToken: Failed to allocate header_b64\n");
    return false;
  }
  memcpy(header_b64, token, headerLen);
  header_b64[headerLen] = '\0';
  
  char* header = (char*)malloc(128);
  if (!header) {
    Serial.printf("JWTHelper::verifyToken: Failed to allocate header\n");
    free(header_b64);
    return false;
  }
  size_t headerDecodedLen = base64UrlDecode(header_b64, (uint8_t*)header, 128);
  free(header_b64);  // Free immediately after use
  if (headerDecodedLen == 0) {
    Serial.printf("JWTHelper::verifyToken: Header decode failed\n");
    free(header);
    return false;
  }
  header[headerDecodedLen] = '\0';
  Serial.printf("JWTHelper::verifyToken: Header decoded: %s\n", header);
  free(header);  // Free header immediately after logging
  
  // Decode payload - extract payload part first (between first and second dot)
  char* payload_b64 = (char*)malloc(payloadLen + 1);
  if (!payload_b64) {
    Serial.printf("JWTHelper::verifyToken: Malloc failed for payload_b64\n");
    return false;
  }
  memcpy(payload_b64, dot1 + 1, payloadLen);
  payload_b64[payloadLen] = '\0';
  
  // Decode payload - use heap allocation to reduce stack usage
  char* payload = (char*)malloc(512);
  if (!payload) {
    Serial.printf("JWTHelper::verifyToken: Malloc failed for payload\n");
    free(payload_b64);
    return false;
  }
  size_t payloadDecodedLen = base64UrlDecode(payload_b64, (uint8_t*)payload, 512);
  if (payloadDecodedLen == 0) {
    Serial.printf("JWTHelper::verifyToken: Payload decode failed (payload_b64 len: %u)\n", payloadLen);
    free(payload_b64);
    free(payload);
    return false;
  }
  payload[payloadDecodedLen] = '\0';
  Serial.printf("JWTHelper::verifyToken: Payload decoded, len=%u: %.200s\n", payloadDecodedLen, payload);
  
  free(payload_b64);  // Free the base64 payload buffer
  
  // Parse payload JSON - use heap allocation to avoid stack overflow in MQTT task
  DynamicJsonDocument* doc = new DynamicJsonDocument(512);
  if (!doc) {
    Serial.printf("JWTHelper::verifyToken: Failed to allocate JSON document\n");
    free(payload);
    return false;
  }
  DeserializationError error = deserializeJson(*doc, payload);
  free(payload);  // Free immediately after parsing
  if (error) {
    Serial.printf("JWTHelper::verifyToken: JSON parse error: %s\n", error.c_str());
    delete doc;
    return false;
  }
  
  // Extract public key
  if (!doc->containsKey("publicKey")) {
    Serial.printf("JWTHelper::verifyToken: Missing publicKey\n");
    delete doc;
    return false;
  }
  const char* pubkey_str = (*doc)["publicKey"];
  if (!pubkey_str) {
    Serial.printf("JWTHelper::verifyToken: publicKey is null\n");
    delete doc;
    return false;
  }
  size_t pubkey_len = strlen(pubkey_str);
  if (pubkey_len != 64) {
    Serial.printf("JWTHelper::verifyToken: publicKey length=%u (expected 64)\n", pubkey_len);
    delete doc;
    return false;
  }
  strncpy(extracted_public_key, pubkey_str, extracted_key_size - 1);
  extracted_public_key[extracted_key_size - 1] = '\0';
  Serial.printf("JWTHelper::verifyToken: Extracted pubkey: %.64s\n", extracted_public_key);
  
  // Extract nonce if present
  if (extracted_nonce && nonce_size > 0) {
    if (doc->containsKey("nonce")) {
      const char* nonce_str = (*doc)["nonce"];
      if (nonce_str) {
        strncpy(extracted_nonce, nonce_str, nonce_size - 1);
        extracted_nonce[nonce_size - 1] = '\0';
      } else {
        extracted_nonce[0] = '\0';
      }
    } else {
      extracted_nonce[0] = '\0';
    }
  }
  
  // Extract timestamps
  if (issued_at) {
    *issued_at = doc->containsKey("iat") ? (*doc)["iat"].as<unsigned long>() : 0;
  }
  if (expires_at) {
    *expires_at = doc->containsKey("exp") ? (*doc)["exp"].as<unsigned long>() : 0;
  }
  
  // Free JSON document immediately after extracting all needed data
  delete doc;
  
  // Check expiration if present
  if (expires_at && *expires_at > 0) {
    unsigned long current_time = time(nullptr);
    if (current_time > 0 && current_time >= *expires_at) {
      return false;  // Token expired
    }
  }
  
  // Convert extracted public key to bytes - use heap allocation to reduce stack usage
  uint8_t* pubkey_bytes = (uint8_t*)malloc(PUB_KEY_SIZE);
  if (!pubkey_bytes) {
    Serial.printf("JWTHelper::verifyToken: Failed to allocate pubkey_bytes\n");
    return false;
  }
  if (!mesh::Utils::fromHex(pubkey_bytes, PUB_KEY_SIZE, extracted_public_key)) {
    Serial.printf("JWTHelper::verifyToken: Failed to convert public key from hex\n");
    free(pubkey_bytes);
    return false;
  }
  Serial.printf("JWTHelper::verifyToken: Public key converted to bytes\n");
  
  // If expected_public_key provided, verify it matches
  if (expected_public_key && key_len == PUB_KEY_SIZE) {
    if (memcmp(pubkey_bytes, expected_public_key, PUB_KEY_SIZE) != 0) {
      Serial.printf("JWTHelper::verifyToken: Public key mismatch\n");
      free(pubkey_bytes);
      return false;
    }
  }
  
  // Decode signature - extract signature part first
  // Signature can be either base64url-encoded (86-88 chars) or hex-encoded (128 chars for 64 bytes)
  char* sig_encoded = (char*)malloc(signatureLen + 1);
  if (!sig_encoded) {
    Serial.printf("JWTHelper::verifyToken: Failed to allocate sig_encoded\n");
    free(pubkey_bytes);
    return false;
  }
  memcpy(sig_encoded, dot2 + 1, signatureLen);
  sig_encoded[signatureLen] = '\0';
  Serial.printf("JWTHelper::verifyToken: Extracted signature (len=%u): %s\n", signatureLen, sig_encoded);
  
  uint8_t* signature = (uint8_t*)malloc(64);
  if (!signature) {
    Serial.printf("JWTHelper::verifyToken: Failed to allocate signature buffer\n");
    free(sig_encoded);
    free(pubkey_bytes);
    return false;
  }
  size_t sigDecodedLen = 0;
  
  // Check if signature is hex-encoded (128 hex chars = 64 bytes) or base64url-encoded (86-88 chars)
  bool is_hex = (signatureLen == 128);  // 64 bytes * 2 = 128 hex chars
  bool looks_like_hex = true;
  if (is_hex) {
    // Verify it's all hex characters
    for (size_t i = 0; i < signatureLen; i++) {
      char c = sig_encoded[i];
      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
        looks_like_hex = false;
        break;
      }
    }
  } else {
    looks_like_hex = false;
  }
  
  if (is_hex && looks_like_hex) {
    // Decode from hex
    Serial.printf("JWTHelper::verifyToken: Signature appears to be hex-encoded\n");
    if (!mesh::Utils::fromHex(signature, 64, sig_encoded)) {
      Serial.printf("JWTHelper::verifyToken: Hex signature decode failed\n");
      free(sig_encoded);
      free(signature);
      free(pubkey_bytes);
      return false;
    }
    sigDecodedLen = 64;
  } else {
    // Try base64url decode
    Serial.printf("JWTHelper::verifyToken: Signature appears to be base64url-encoded\n");
    sigDecodedLen = base64UrlDecode(sig_encoded, signature, 64);
    if (sigDecodedLen != 64) {
      Serial.printf("JWTHelper::verifyToken: Base64url signature decode failed, got %u bytes (expected 64, sig_encoded len: %u)\n", 
                    sigDecodedLen, signatureLen);
      Serial.printf("JWTHelper::verifyToken: Signature string: %s\n", sig_encoded);
      free(sig_encoded);
      free(signature);
      free(pubkey_bytes);
      return false;
    }
  }
  
  free(sig_encoded);  // Free immediately after decoding
  
  Serial.printf("JWTHelper::verifyToken: Signature decoded successfully (%u bytes)\n", sigDecodedLen);
  
  // Create signing input: header.payload
  size_t signingInputLen = headerLen + 1 + payloadLen;
  if (signingInputLen >= 1024) {
    Serial.printf("JWTHelper: Signing input too large: %u bytes\n", signingInputLen);
    free(signature);
    free(pubkey_bytes);
    return false;  // Signing input too large
  }
  char* signingInput = (char*)malloc(signingInputLen + 1);
  if (!signingInput) {
    Serial.printf("JWTHelper: Failed to allocate signing input buffer\n");
    free(signature);
    free(pubkey_bytes);
    return false;
  }
  
  memcpy(signingInput, token, headerLen);
  signingInput[headerLen] = '.';
  memcpy(signingInput + headerLen + 1, dot1 + 1, payloadLen);
  signingInput[signingInputLen] = '\0';
  
  Serial.printf("JWTHelper: Verifying signature, signingInputLen=%u, pubkey=%.64s\n", 
                signingInputLen, extracted_public_key);
  
  // Verify signature - feed watchdog before potentially long operation
#ifdef ESP_PLATFORM
  yield();  // Feed watchdog before verification
#endif
  
  int verify_result = ed25519_verify(signature, (const unsigned char*)signingInput, signingInputLen, pubkey_bytes);
  
#ifdef ESP_PLATFORM
  yield();  // Feed watchdog after verification
#endif
  
  Serial.printf("JWTHelper: ed25519_verify result: %d (1=success, 0=fail)\n", verify_result);
  
  // Free all allocated buffers
  free(signingInput);
  free(signature);
  free(pubkey_bytes);
  
  return (verify_result == 1);
}
