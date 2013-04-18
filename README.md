# Build Notes
1. CMake 2.8.10 or greater required, for use of objects

# Attic

Attic is a Tent app that syncs the contents of a folder across multiple devices
and operating systems. All files are encrypted individually.

## Master Key

Each user has a randomly generated *master key* that is used to encrypt *file
keys*. The *master key* is encrypted with AES-256-CFB using the *user key* and
stored in the Tent profile section for Attic.

## User Key

The *user key* is derived from a user-supplied password using
[scrypt](http://www.tarsnap.com/scrypt.html), with a salt stored in the Tent
profile section for Attic.

## Process

1. File is created/modified.
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

#### File metadata post

`https://cupcake.io/types/post/attic-file/v0.1.0`

Property | Required | Type | Description
-------- | -------- | ---- | -----------
name | Required | String | Name of the file
size | Required | Integer | The filesize in bytes of the uncompressed file.
type | Optional | String | The MIME type of the file, if known (using [libmagic](https://en.wikipedia.org/wiki/Libmagic) and file extension heuristics).
keydata | Required | String | The base64 encoded encryption key and authentication key for the file. This data is encrypted using the master key.
chunk_ids | Required | Array | An array of chunk identifiers that should be concatenated together to recreate the file
chunk_posts | Required | Array | An array of the identifiers of the posts containing chunk attachments for this file

#### Chunk store post

`https://cupcake.io/types/post/attic-chunks/v0.1.0`

This post has one field, `chunks`, an array of objects; one for each chunk attached to the post with three fields:

- `plaintext_mac`: The plaintext authenticator, hex encoded.
- `ciphertext_mac`: The ciphertext authenticator, hex encoded.
- `iv`: The base64 encoded IV used to encrypt the chunk.

Each chunk is an attachment, the filename is the hex encoded plaintext
authenticator. The content-type should be set to `application/octet-stream`.
Each post should be capped at 100 chunks.

#### Folder post

`https://cupcake.io/types/post/attic-folder/v0.1.0`

_depricated_
This post has one field, `children`, which is an object. The keys are the names
of the children (files and folders) of the folder. The values are an object with
two fields:

Folder posts no longer contain information about what's in them, they conatain their
relative directory path and act as an anchor for file posts to mention. 

`id`: The post id of the object.

_depricated_
`type: Either `file` or `folder`.

#### Profile section 

_depricated_
`https://cupcake.io/types/profile/attic/v0.1.0`

- `master_key`: The base64 encoded encrypted *master key* used by the entity.
- `user_key_salt`: The base64 encoded salt for the *user key*.
- `root_folder`: The identifier of the root folder post.


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

## Local Directories

In the MVP only a single folder will be selected for use with Attic.

## Clients

### Native

Most of the business logic will be implemented in a portable C++ library that is cross-platform.

Native clients will be created for:

 - **Mac OS X**
 - Windows (7)
 - Linux
 - **iOS**
 - Android
 
Native clients will keep a folder on the host filesystem in sync across all
devices. The UI should be minimal and consist of two menus, one on the system
level:

 - toggle sync (on/off)
 - connect to Tent

and one on right click:

 - share file
 - make file public
 - stop/start syncing file
 
A **web client** will also be built. The web client will provide an
administrative interface allowing users to choose between conflicted file
versions and share files with other users.


## Division of Labor

 - Core library - **Manny**
 - Crypto strategy - **Jonathan**
 - OS X app - **Vince**
 - Web app - **Jesse**

## Library Interaction Abstract

### Core API


int StartupAppInstance( const char* szAppName,
                        const char* szAppDescription,
                        const char* szUrl,
                        const char* szIcon,
                        char* redirectUris[],
                        unsigned int uriCount,
                        char* scopes[],
                        unsigned int scopeCount);


int InitLibAttic( const char* szWorkingDirectory,
                  const char* szConfigDirectory,
                  const char* szTempDirectory,
                  const char* szEntityURL,
                  unsigned int threadCount = 2);

int ShutdownLibAttic();

// Master Key
int EnterPassphrase(const char* szPass);
int RegisterPassphrase(const char* szPass, bool override = false);
int ChangePassphrase(const char* szOld, const char* szNew);
int GetPhraseStatus();

// Pass the uri to the api path for apps (ex "https://test.tent.is/tent/app")
int RegisterApp(const char* szPostPath);

// Pass the api root of the entity (ex "https://test.tent.is/tent/")
// * Must Register app successfully before proceeding to this step
int RequestAppAuthorizationURL(const char* szApiRoot);

const char* GetAuthorizationURL();

int RequestUserAuthorizationDetails(const char* szApiRoot, const char* szCode);

// Save the app in json to a file (Just a utility you probably don't
// want to use this in production)
int SaveAppToFile();

// Load the app in json from a file (Just a utility you probably don't
// want to use this in production)
int LoadAppFromFile();
int LoadAccessToken();

// Pushfile to tent
int PushFile(const char* szFilePath, void (*callback)(int, void*));

// Pullfile from tent
int PullFile(const char* szFilePath, void (*callback)(int, void*));

// Delete a file
int DeleteFile(const char* szFileName, void (*callback)(int, void*));

// Pull All files in manifest
int PullAllFiles(void (*callback)(int, void*)); // Pull into lib, don't expose

int SyncFiles(void (*callback)(int, void*));


