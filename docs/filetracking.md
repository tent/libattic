# File Tracking

## Files

Each file is associated with a folder post id. This dictates where it will end up on each device.

## Folders

Each individual folder has its associated folder post, that post contains the name of the folder and the id of it's parent. Folder posts are cached locally and folder paths are constructed on a per need basis. Folder paths are singly linked lists.

### Root Folders

Root folders are folders that contain the contents of your synced data. Acting as aliases on a per device basis. They allow for individual device mapping.

Root folders are stored as an object in the attic config post.
{ <name>, <post id> }

