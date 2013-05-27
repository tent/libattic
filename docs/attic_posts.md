# Attic Posts

## File Post *metadata*

`https://cupcake.io/types/post/attic-file/v0.1.0`

## Chunk Post

`https://cupcake.io/types/post/attic-chunks/v0.1.0`

This post has one field, `chunks`, an array of objects; one for each chunk attached to the post with three fields:

- `plaintext_mac`: The plaintext authenticator, hex encoded.
- `ciphertext_mac`: The ciphertext authenticator, hex encoded.
- `iv`: The base64 encoded IV used to encrypt the chunk.

Each chunk is an attachment, the filename is the hex encoded plaintext
authenticator. The content-type should be set to `application/octet-stream`.
Each post should be capped at 100 chunks.

## Folder Post

`https://cupcake.io/types/post/attic-folder/v0.1.0`

## Credentials Post
