/**
 * File information:
 * 
 * & C++ header file
 * 
 * @details A malloc built for inegrated processors & custom OS development
 * @brief Loosely adapted from various malloc() implementations & OSDev Wiki
 * @warning Does NOT check for upper-bounds of device memory. Make sure to do that yourself!
 ** /

Copyright Avetharun 2021 - GNU-GPL3-a

Let's keep this simple. It follows the GNU GPL3 license, with very few modifications and exceptions.


You can:
  - Use this commercially
  - Distribute modified versions
  - Modify this file and/or projects
  - Patent projects that use this file
  - Use this file and/or project privately, ie. for use in an internal server (See limitations & Exceptions)
  - Use this project freely under the following conditions

Conditions:
  - You must disclose the source file and/or author when distributing this file
  - A copy of this license must be attached to the file, in verbatim
  - The same (or similar) license must be used if you modify and license your version
  - Changes must be listed. ie. modifying source code must have your name and change in the file.
    - To follow this it must either be 
      A: Beside the change (on top or on bottom, in a comment)
      B: In the AUTHORs section of the file/project
      C: in any changelog that references the file's changes

Limitations / What you cannot do
  - The user (you) are not liable for the harmful actions this program or file may cause, if any.
  - Keep code closed source (*See exceptions)
 Exceptions
  - If code is closed source, it must be in a private setting. Examples are as follows:
    EXA: A server used to host and/or distribute files
    EXB: Used as a base for porting and/or in microcontrollers

 */


#if !defined(clearos_os_cstd_alloc_h__)
#define clearos_os_cstd_alloc_h__
#if (!defined(size_t) || defined(NEED_SIZE_T)) && !defined(DONT_NEED_SIZE_T)
typedef unsigned long size_t;
#endif
#if (!defined(uintptr_t) || defined(NEED_UINTPTR_T)) && !defined(DONT_NEED_UINTPTR_T)
typedef unsigned long * uintptr_t;
#endif
#if (!defined(intptr_t) || defined(NEED_INTPTR_T)) && !defined(DONT_NEED_INTPTR_T)
typedef unsigned long * intptr_t;
#endif



typedef struct s_bloc
{
    s_bloc * next;
    s_bloc * prev;
    bool	 isFree;
    size_t   size;
    void *   data;
    char	 __unused_used_for_header_;
    s_bloc(int size) {
        this->size = size;
    }
} m_bloc;


#define __HEADER_SIZE__ sizeof(struct s_bloc)
#define __ALIGN_ADDR__(size, power) (((size - 1) / power + 1) * power)


extern void * malloc( size_t size );
extern void   free(void *);
extern void * calloc(size_t num_members, size_t size);
extern void * realloc(void * ptr, size_t size);
// Copy N bytes from src into dest
extern void memcpy (void* dest, void* src, size_t n);
// Set N bytes of src to value v
extern void memset (void* src, char v, size_t n);
// Set N bytes of src to value v, in intervals of 32 bits
extern void memset (void* sec, long int v, size_t n);


#endif // clearos_os_cstd_alloc_h__
