#Filesystem Namespace:  
---  
**Rules:**  

- Must forward all exceptions  
- Must work with the Standard Template Library directly  
- Error double-check, which will check the actual results of the operation, and return specific error results for:
    - copy
    - move
    - delete
