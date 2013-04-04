# Attic File Synchronization 
    There are two basic cases for file synchronization. Synchronization between devices, both connected to the same entity, and synchronization between entities.

## File Posts
    When a file is sent up to the server, it is sent up in two posts, the chunk post(s), containing the file chunk attachments, and the file post, which contains the file metadata. Each file post is created with a parent (none if it is a new file). This will create a graph of posts, and allow a user to walk the graph.
A---->B---->C--->D
    
## Conflicts
### Detecting conflicts

A---->B---->C--->D

             `-->D(2)

* A conflict can be detected by traversing each post trees leaf nodes. 
    * Paying specific attention to multiple leafs with the same parent. When this happens, there is a potential conflict.
        * A conflict will arise if the two or more files share a common parent, and have the same name.
        * All versions of the file will be dowlnoaded locally and  will be renamed. 
        * The file(s) that will be renamed will be the file that was created by the non originitating entity or device.
    * Files in conflict can be resolved by the user renaming either of the files.
        * This will create a new meta-post, referencing the original parent
            * Still a fork, but with no name conflicts there are no problems.

### Changes to libattic
* Libattic, on a push request, will now have to check all file hashes in local cash against every potentially new file being pushed.
    * This is to detect renames.
        * Primarily for OSX, OSX filesystem currently does not support file modification events. 
            * Note* this will have to be handled differently on linux, which does support these notification events.
* Libattic, will have to be able to traverse a file's lineage to detect possible conflicts. 
    * This will probably mostly be done with partial upwards traversals.
