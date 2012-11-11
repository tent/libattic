# Attic

This Tent app syncs the contents of a folder across multiple devices and operating systems. All files are encrypted individually with unique keys and can be shared publicly or with specific users. 

Each user has a master encryption key that is used to encrypt encryption keys
for each of the files. The master key is derived from the user's passphrase with
[scrypt](http://www.tarsnap.com/scrypt.html), with the salt stored in a private
Tent profile info section.

## Process

1. File is created/modified.
2. File is compressed.
3. File is encrypted.
4. File is broken into chunks.
5. Tent post is created with chunks as attachments.
6. Details about the file are added to the metastore post.

### Compression

Files are compressed with [zlib](http://zlib.net/). Crypto++ provides a zlib
implementation that is easy to work with as a filter.

### Encryption

Each file is encrypted with a randomly generated symmetric key. This allows a single file (and its key) to be shared with another user without compromising other files.

The [Crypto++](http://cryptopp.com/) library is used for encryption and
compression. Files are encrypted with
[AES-256](http://en.wikipedia.org/wiki/Advanced_Encryption_Standard) using the
[GCM](https://en.wikipedia.org/wiki/Galois/Counter_Mode) block cipher mode,
providing data authenticity and confidentiality. The nonce and key are randomly
generated for each file.

The key for each file is encrypted using the master key with a random nonce and
stored in the metastore.

### Chunking

The encrypted file is broken into 4MB chunks and attached to a Tent post.

### Metastore

The metastore is a [sqlite](http://sqlite.org/index.html) database containing
a database of the file metadata and encrypted symmetric keys, stored in
a separate post (see [Post Types](#post-types)).

#### Tables

The hierarchy is represented relationally using the *nodes* table. Each file and directory maps to a row in that table. File nodes also connect with the *files* table via `file_id`.

**nodes**

Column | Type | Required | Description
------ | ---- | -------- | -----------
id | Integer | Required | id of node
parent_id | Integer | Optional | id of parent node
file_id | Integer | Optional | id of file (when NULL, it's a directory)
name | Text | Required | name of node (file or directory)


**files**

Column | Type | Required | Description
------ | ---- | -------- | -----------
id | Integer | Required | id of file
post_id | Text | Required | id of Tent post
post_version | Integer | Required | version of Tent post
size | Integer | Required | Size of file in bytes
type | Text | Optional | MIME type of file if known
key | Blob | Required | encryption key (32 bytes)
nonce | Blob | Required | encryption Initialization Vector
updated_at | Date | Required | epoch timestamp

## Update/Sync

### New files

New Tent posts and versions/updates of type (attic?) since last sync should be queried, compared to current file versions, and downloaded to the device if they are more recent.

### Version conflicts

Two versions of a file are in conflict if they have each been altered and one is not the ancestor of the other. In this case, both should be saved and the user presented with a choice between them. Real time collaborative editing is outside the scope of Attic.

## Sharing/Privacy

There are three layers of sharing:

Layer | Encrypted | Updates Shared | Visible to randoms
------------ | ------------- | ------------ | ------------
Private | Yes | No | No
Shared | Yes (key shared) | Yes | No
Public | No | Yes | Yes

## Local Directories

In the MVP only a single folder will be selected for use with Attic.

## Posts

A post is created for each file. Changes to the file, it's name, and location are stored as versions of the post.

### Post Types

The primary file storage post type: `https://tent.io/types/post/attic/v0.1.0`

Property | Required | Type | Description
------------ | ------------- | ------------ | ----------
name | Required | String | Name of the file
path | Required | String | Relative location of the file within the *Attic* folder
size | Required | Integer | The filesize in bytes of the uncompressed file.
type | Optional | String | The MIME type of the file, if known (using [libmagic](https://en.wikipedia.org/wiki/Libmagic) and file extension heuristics).

Metastore (file index/keystore) post type (see [Metastore](#metastore)): `https://tent.io/types/post/attic-metastore/v0.1.0` (**WIP**)

Property | Required | Type | Description
-------- | -------- | ---- | ----------
attic_root | Required | String | Filesystem path to Attic directory (e.g. `/Users/example/Attic`)

[sqlite](http://sqlite.org/index.html) DB file should be an attachment with *metastore* as the name.

We may need additional post types for:

1. Sharing/sync with other users
2. Files sharded across multiple posts

## API changes

Some changes need to be made to the Tent to support Attic. They include:

- Sort posts/post versions by `updated_at` while using `before_time`/`since_time` (e.g. fetch all posts/post versions created/updated after my last fetch time.)

(Jesse and Jonathan, please fill in some info here.)

## Clients

### Native

Most of the business logic will be implemented in a portable C++ library that is cross-platform.

Native clients will be created for:

 - **Mac OS X**
 - Windows (7)
 - Linux
 - **iOS**
 - Android
 
Native clients will keep a folder on the host filesystem in sync across all devices. The UI should be minimal and consist of two menus, one on the system level:

 - toggle sync (on/off)
 - connect to Tent

and one on right click:

 - share file
 - make file public
 - stop/start syncing file
 
A **web client** will also be built. The web client will provide an administrative interface allowing users to choose between conflicted file versions and share files with other users.


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

