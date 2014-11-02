#Filesystem Namespace:  
---  
**Coding guidelines:**  
 
-  The abstraction should forward all exceptions with the location which the error occured. 
-  The abstraction should prefer the use of the STL over Boost alternatives (aka: instead of a path, use a string).  This guideline really only applies to return values of the abstraction, however, there may be other cases where the STL is preferred over boost. 
-  All errors should have an explanation that the user can either recieve in an exception that will be displayed, or in a string that can be used to determine whether the error can be resolved.
