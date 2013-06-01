# Libattic
Libattic encapsulates base attic functionality. This includes uploading, downloading, chunkig, and ecrypting files. Attic
provides a means of storing encrypted files in tent posts.

## Keys
### Master Key

Each user has a generated *master key* that is used to encrypt *file
keys*. The *master key* is encrypted with AES-512-CFB using the *user key* and
stored in the Tent profile section for Attic.

### File Key
Each file upon first upload is generated a unique key associated with it's corresponding post. Each file key is encrypted
with the user's master key. This allows us, if needed, to re-encrpyt the keys without having to generate and re-encrypt
files.
 
### User Key *Passphrase*

The *user key* is derived from a user-supplied password using
[scrypt](http://www.tarsnap.com/scrypt.html), with a salt stored in the Tent
profile section for Attic.

## Process

1. Library is called, with filepath to file.
2. File is broken into chunks.
3. Chunks are compressed.
4. Chunks are encrypted.
5. Tent posts are created with chunks as attachments.
6. Details about the file are added to the metadata posts.

### Chunking

Files are chunked using a [rolling
hash](https://en.wikipedia.org/wiki/Rolling_hash) that determistically breaks
files into chunks around "interesting" boundaries. This means that if part of
a binary file is changed, only the parts that have been modified need to be
uploaded.

The hash algorithm is based on [the
one](https://github.com/apenwarr/bup/blob/master/lib/bup/bupsplit.c) used in
[bup](https://github.com/apenwarr/bup). There is also a [Go
implementation](http://camlistore.org/code/?p=camlistore.git;a=blob_plain;f=pkg/rollsum/rollsum.go;h=4a66d70551bbcd3fd307c50eb447f6a0a203f0dc;hb=HEAD)
in [camlistore](http://camlistore.org/). The [bup
docs](https://github.com/apenwarr/bup/blob/master/DESIGN#L92) describe how this
works on a theoretical level. Attic simplifies the approach by removing the tree
of splits, and changing the blob size target to 4MB by requiring the lowest 22
bits of the hash to be ones 2^22 = 4194304 = ~4.1MB. Additionally a minimum
chunk size of 1MB is enforced (if the hash split triggers and the chunk is under
1MB, continue to the next boundary).

This means that each file version is composed of a list of chunks. If the
file is modified, it's likely that only some of the chunks will change, reducing
the need to re-upload the entire file.

Each chunk is individually processed through the
compression/encryption/authentication/upload pipeline.

### Compression

Chunks are compressed with zlib.

### Encryption

Each file has a randomly generated *file key*. This allows a single file (and
its key) to be shared with another user without compromising other files.

Each chunk is symmetrically encrypted using the *file key* and a randomly
generated IV. The IV **must** be unique for each chunk of a file.

Chunks are encrypted with
[AES-256](http://en.wikipedia.org/wiki/Advanced_Encryption_Standard) using the
[CFB](http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation#Cipher_feedback_.28CFB.29) block cipher mode.

### Authentication

Each file has a randomly generated *authentication key* used to authenticate file
chunks. There are two authenticators, the plaintext authenticator and the
ciphertext authenticator.

#### Plaintext Authenticator

The plaintext authenticator is a HMAC-SHA256 of the chunk plaintext before it
has been compressed. This MAC uniquely identifies the chunk, and is used as its
attachment filename and in the chunklist for the file.

#### Ciphertext authenticator

The ciphertext authenticator is a HMAC-SHA256 of the IV used for the chunk
concatenated with the the ciphertext. This MAC is used to make sure that the
encrypted chunk has not been tampered with.

### Upload

Files are stored in Tent as a set of posts that contain the file metadata and
chunks. Each file has one post that contains the metadata and keys for the file.
The chunks are stored as attachments to a series of posts for each file.

## Update/Sync

### New files

New Tent posts and versions/updates of type (attic?) since last sync should be
queried, compared to current file versions, and downloaded to the device if they
are more recent.

### Version conflicts

Two versions of a file are in conflict if they have each been altered and one is
not the ancestor of the other. In this case, both should be saved and the user
presented with a choice between them. Real time collaborative editing is outside
the scope of Attic.

## Sharing/Privacy

There are three layers of sharing:

Layer | Encrypted | Updates Shared | Visible to randoms
------------ | ------------- | ------------ | ------------
Private | Yes | No | No
Shared | Yes (key shared) | Yes | No
Public | No | Yes | Yes
