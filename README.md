# lo_custom_encryption

Sample extension to demonstrate LibreOffice 7.0 [XPackageEncryption](https://api.libreoffice.org/docs/idl/ref/interfacecom_1_1sun_1_1star_1_1packages_1_1XPackageEncryption.html) interface.

Interface _XPackageEncryption_ allows to implement new encryption methods (_dataspaces_ in terms of [MS-OFFCRYPTO](https://docs.microsoft.com/en-us/openspecs/office_file_formats/ms-offcrypto/51a47a05-73a2-4e2b-b7ee-f7b4bcb8876d)) to use with documents. One default implementation for password protected documents (_"StrongEncryptionDataSpace"_) already embedded into LibreOffice core. But there can be more of them, for example _"DRMEncryptedDataSpace"_.

This sample extension implements new custom encryption service _"XorEncryptedDataSpace"_ that does primitive [XOR](https://en.wikipedia.org/wiki/XOR_cipher) to demonstrate features of this interface. It can encrypt documents on save and decrypt documents using this encryption type on load.
