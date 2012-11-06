#Attic

This Tent app syncs the contents of a folder across multiple devices and operating systems. All files are encrypted individually with unique keys and can be shared publicly or with specific users. 

## Process

1. File is created/modified.
2. File is compressed.
3. File is encrypted.
4. File is broken into chunks.
5. Tent post is created with chunks as attachments.

### Compression

Files will be compressed with [zlib](http://zlib.net/).

### Encryption

Each file is encrypted with a unique symmetric key pair by default. This allows a single file (and its key) to be shared with another user without compromising other files. The symmetric keys for files will be stored in the "keystore" post, encrypted with another key derived from the users passphrase.

### Chunking

Files will be broken into 4MB chunks (subject to change).

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

This folder will also include a SQLite database containing a list of posts to be included in the folder and their current versions.

## Posts

A post is created for each file. Changes to the file, it's name, and location are stored as versions of the post.

### Post Types

The primary file storage post type:

Property | Required | Type | Description
------------ | ------------- | ------------ | ----------
name | Required  | String | Name of the file
file_path | Required  | String | Relative location of the file within the *Attic* folder
sha256 | Required | String | The hexadecimal SHA-256 hash of the file before compression/encryption

We may need additional post types for:

1. Sharing/sync with other users
2. Files sharded across multiple posts
3. The file index/keystore

## API changes

Some changes need to be made to the Tent to support Attic. They include:

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
