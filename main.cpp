#include <Windows.h>
#include <cstdio>

int AlignedSize(int size, int alignment) {
	return ((size + alignment - 1) / alignment) * alignment;
}

BOOL GenerateAESKey(HCRYPTKEY* aes_key) {
	HCRYPTKEY crypt_provider;

	if (!CryptAcquireContextA(&crypt_provider, NULL, NULL, PROV_RSA_AES,
		CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
		printf("CryptAcquireContextA() failed, error: %X", GetLastError());

		return FALSE;
	}

	if (!CryptGenKey(crypt_provider, CALG_AES_128, CRYPT_EXPORTABLE, aes_key)) {
		printf("CryptGenKey() failed, error: %X", GetLastError());

		return FALSE;
	}

	return TRUE;
}

BOOL WriteEncryptedFile(HCRYPTKEY aes_key, const char* file_name, const char* data, size_t data_size) {
	FILE *f;
	DWORD out_size = data_size;
	DWORD aligned_size = AlignedSize(data_size, 16) + 16;

	BYTE* encrypt_buffer = (BYTE*)malloc(aligned_size);
	if (!encrypt_buffer) {
		return FALSE;
	}
	memcpy(encrypt_buffer, data, data_size);
	
	if (!CryptEncrypt(aes_key, NULL, TRUE, 0, encrypt_buffer, &out_size, aligned_size)) {
		printf("CryptEncrypt() failed, error: %X\n", GetLastError());

		return FALSE;
	}
	fopen_s(&f, file_name, "wb");

	if (!f) {
		printf("Failed to open file, error: %X\n", GetLastError());

		return FALSE;
	}
	fwrite(encrypt_buffer, out_size, 1, f);
	fclose(f);

	return TRUE;
}


void main() {
	HCRYPTKEY aes_key;

	const char* write_array[3][2] = {
		{"This is a secret", "1.txt"},
		{"This is also a secret", "2.txt"},
		{"You gotta be kidding me", "3.txt"}	
	};

	if (GenerateAESKey(&aes_key)) {
		for (unsigned int i = 0; i < sizeof(write_array) / sizeof(write_array[0]); i++) {

			size_t data_length = strlen(write_array[i][0]);
			const char* data = write_array[i][0];
			const char* file_name = write_array[i][1];

			if (WriteEncryptedFile(aes_key, file_name, data, data_length)) {

				printf("successfully wrote to encrypted file %s!\n", file_name);
			}
		}
	}
 	getchar();
}
