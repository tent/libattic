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
    * Paying specific attention to multiple leafs with the same parent. When this happens, the conflict has to be resolved. 
    * Both versions of the file will be dowlnoaded locally and one will be renamed. 
        *The file that will be renamed will be the file that was created by the non originitating entity or device.
