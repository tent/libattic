# Attic

This Tent app syncs the contents of a folder across multiple devices and
operating systems. All files are encrypted individually with unique keys and can
be shared publicly or with specific users. 

Each user has a master encryption key that is used to encrypt encryption keys
for each of the files. The master key is derived from the user's passphrase with
[scrypt](http://www.tarsnap.com/scrypt.html), with the salt stored in a private
Tent profile info section.

## Process

1. File is created/modified.
2. File is broken into chunks.
3. File is compressed.
4. File is encrypted.
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

Each file is encrypted with a randomly generated symmetric key. This allows
a single file (and its key) to be shared with another user without compromising
other files.

Each chunk is encrypted with the file key and a randomly generated IV. The IV
*must* be unique for each chunk of a file.

Files are encrypted with
[AES-256](http://en.wikipedia.org/wiki/Advanced_Encryption_Standard) using the
[CFB](http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation#Cipher_feedback_.28CFB.29) block cipher mode.

### Authentication

Each file has a randomly generated authentication key used to authenticate file
chunks. There are two authenticators, the plaintext authenticator and the
ciphertext authenticator.

#### Plaintext Authenticator

The plaintext authenticator is a HMAC-SHA256 of the chunk before it has been
compressed. This MAC uniquely identifies the chunk, and is used as its
attachment filename and in the chunklist for the file.

#### Ciphertext authenticator

The ciphertext authenticator is a HMAC-SHA256 of the IV used for the chunk and
the ciphertext. This MAC is used to make sure that the encrypted chunk has not
been tampered with.

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

`plaintext_mac`: The plaintext verifier, hex encoded.
`ciphertext_mac`: The ciphertext verifier, hex encoded.
`iv`: The IV used to encrypt the chunk.

Each chunk is an attachment, the filename is the hex encoded plaintext verifier.
The content-type should be set to `application/octet-stream`. Each post should
be capped at 100 chunks.

#### Folder post

`https://cupcake.io/types/post/attic-folder/v0.1.0`

This post has one field, `children`, which is an object. The keys are the names
of the children (files and folders) of the folder. The values are an object with
two fields:

`id`: The post id of the object.
`type: Either `file` or `folder`.


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
 - **StartupAttic()**
    - This method will essentially take care of all necessary start up functionality of the attic.
    - This includes:
        -  Initially requesting the latest metadata and sqlite files.
        -  Continuing any previously interrupted uploads.
    - This returns a pointer to manager
 - **ShutdownAttic(terminate=false)**
    - This will shutdown the lib
        - it will attempt to finish whatever it was doing before cleanup
        - if terminate is true it will break out of the process and end it all.
 - **SyncFile(wstring& filepath)**
    - Pass in a filepath to a file (preferably absolute)
    - wide string (utf-16)
    - The lib will queue it for processing
        - processing includes
            - syncing metadata
            - compressing the file
            - chunking
            - encrypting
        - and then send it off to the tent server
    - This will return a handle to the file's particular processing information
        - Which can be used to check the status of a particular file.
 - **CancelSync(filehandle)**
    - This will terminate the process and a cleanup function will be called
 - **PauseSyncing()**
    - This will pause the entire syncing process
    - No files will be processed.
    - Files can still be inserted into the queue
 - **UnpauseSyncing()**
    - Syncing resumes
 - **GetFileStatus(filehandle)**
    - returns a struct filled with information pertaining to an actual file.
 - **GetAtticStatus()**
    - returns a structed filled with infomration pertaining to attic itself
    - number in queue
    - current operation
 - **ForceSync()**
    - Forces the sync process, retrieving necessary metadata
 - **Authenticate(entityuri)**
    - begins the process to retrieve authentication credentials
    - returns url the user needs to go to to complete app authentication
 - **TradeInCode(code)**
    - finish the O-auth process, trade in code for auth keys
 - **SetPassphrase(phrase)**
    - set's the user's passphrase that all encryption keys are based on
 - **ChangePassphrase(newphrase)**
    - changes encryption keys
    - updates all metadata accordingly
