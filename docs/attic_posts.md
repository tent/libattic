# Attic Posts

## File Post *metadata*

`https://attic.is/types/file/v0`
The file post type describes the metadata of a file. It does not store chunks.
- `name` : name of the file - *string*
- `path` : relative path of a file - *string*
 

## Chunk Post

`https://attic.is/types/chunk/v0`
The chunk post stores chunks of a file as attachments, it also stores iv data of a file used in correlation with the 
file key in the metapost for the decryption step. It is possible to have multiple chunk posts per file. If a file is very
large, chunks are grouped in separate posts. Chunk posts `mention` **mention what**

- `plaintext_mac`: The plaintext authenticator, hex encoded.
- `ciphertext_mac`: The ciphertext uthenticator, hex encoded.
- `iv`: The base64 encoded IV used to encrypt the chunk.

Each chunk is an attachment, the filename is the hex encoded plaintext
authenticator. The content-type should be set to `application/octet-stream`.
Each post should be capped at 100 chunks.

## Folder Post

`https://attic.is/types/folder/v0`
Folder posts track folder path, and are mentioned by fileposts as an anchor. When a file is downloaded ultimately the
folderpost it's associated with will be the final deciding factor for its actual filepath on a system. This also 
allows a user to query by mention for every filepost in a folder.

## Credentials Post

`https://attic.is/types/cred/v0`
This post retains the encrypted master key associated with an entity, there can be only one. 
