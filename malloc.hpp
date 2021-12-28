/**
 * File information:
 * 
 * & C++ header-only file
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


#if !defined(clearos_os_cstd_malloc_hpp)
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



m_bloc * base_bloc = nullptr;


int __internal_page_size = 32; // bytes

uintptr_t * __internal_brk = 0;
uintptr_t * __internal_data_seg_end = (uintptr_t * )0xffff;

int __get_pagesize_() { return __internal_page_size; }
void __set_pagesize_(int size) { __internal_page_size = size; }

// Increase data space by <increment> bytes. 
// Pass 0 through this function to get current limit
void* sbrk(int incrememt) {
    if (incrememt > 0) { __internal_brk += incrememt; }
    return __internal_brk; 
}
// Set program data segment upper limit (used for malloc)
uintptr_t * brk(void * addr) {
    uintptr_t * last = __internal_brk;
    __internal_brk = (uintptr_t * )addr;
    return last;
}

// Find first free block of memory, or return nullptr if none is found
m_bloc * __find_free_bloc_(m_bloc * block, size_t req_amt) {
    while (block != nullptr) {
        if (block->isFree == true && block->size >= req_amt) {
            return block;
        }
        block = block->next;
    }
    return nullptr;
}

m_bloc * get_bloc(void *ptr) {
    if (ptr < sbrk(0) && ptr >= base_bloc)
        return (m_bloc * ) (ptr = (char*)ptr - __HEADER_SIZE__);
    return (nullptr);
}

void create_rest_bloc(m_bloc *bloc, size_t size, int pagesize) {
    m_bloc	*rest;
    rest = (m_bloc*)((size_t)bloc->data + bloc->size);
    rest->size = (__ALIGN_ADDR__(size + __HEADER_SIZE__, pagesize)
		    - bloc->size - (__HEADER_SIZE__ * 2));
    rest->isFree = true;
    rest->data = (char*)rest + __HEADER_SIZE__;
    rest->next = nullptr;
    rest->prev = bloc;
    if (bloc == nullptr) { return; }
    bloc->next = rest;
}
m_bloc __dummy_block = m_bloc(16);
m_bloc * create_dummy_bloc() {
    sbrk(64);
    m_bloc	*rest = &__dummy_block;
    rest->size = 16;
    rest->size = (__ALIGN_ADDR__(16 + sizeof(struct s_bloc), 16)
		    - 16 - (sizeof(struct s_bloc) * 2));
    rest->isFree = true;
    rest->data = (char*)rest + __HEADER_SIZE__;
    rest->next = nullptr;
    rest->prev = rest;
    return rest;
}



m_bloc * extend_heap(m_bloc *last_bloc, size_t size) {
    m_bloc *bloc;
    int    pagesize;
  
    pagesize = __get_pagesize_();
    
    if ((bloc = (m_bloc * )
        sbrk(__ALIGN_ADDR__(size + __HEADER_SIZE__, pagesize))) == (void*)-1) {
            return (nullptr);
        }
    
    if (last_bloc != nullptr) {
        last_bloc->next = bloc;
        bloc->prev = last_bloc;
      }
    else {
        bloc->prev = nullptr;
    }
    bloc->size = size;
    bloc->isFree = false;
    bloc->data = (char*)bloc + __HEADER_SIZE__;
    //create_rest_bloc(bloc, size, pagesize);
    return (bloc);
}



int split_bloc(m_bloc * this_bloc, size_t size) {
    m_bloc * half_bloc;
  
    if (this_bloc->size <= (size + __HEADER_SIZE__)) {
        return (1);
    }
    half_bloc = (m_bloc*)((char*)this_bloc + __HEADER_SIZE__ + size);
    half_bloc->size = this_bloc->size - size - __HEADER_SIZE__;
    half_bloc->next = this_bloc->next;
    half_bloc->prev = this_bloc;
    half_bloc->isFree = true;
    half_bloc->data = (char*)half_bloc + __HEADER_SIZE__;
    this_bloc->size = size;
    this_bloc->next = half_bloc;
    this_bloc->isFree = false;
    return (0);
}

m_bloc * fusion_free_bloc(m_bloc *bloc) {
    m_bloc	*tmp;
  
    tmp = bloc;
    if (bloc->prev != nullptr && bloc->prev->isFree == true) {
        bloc->prev->next = bloc->next;
        bloc->next->prev = bloc->prev;
        bloc->size += bloc->prev->size + __HEADER_SIZE__;
        tmp = bloc->prev;
    }
    if (tmp->next != nullptr && tmp->next->isFree == true) {
        tmp->size += tmp->next->size + __HEADER_SIZE__;
        tmp->next = tmp->next->next;
    }
    return (tmp);
}



// Allocate <size> memory in bytes
void * malloc( size_t size ) {
    m_bloc *this_bloc;
    size = __ALIGN_ADDR__( size, 16 );
    if (base_bloc == nullptr) {
        // First malloc() call, create a "dummy" block
        base_bloc = create_dummy_bloc();
        base_bloc->next = this_bloc;
    }
    // Find first free bloc of size. If none is found, create a new one.
    if ((this_bloc = __find_free_bloc_(base_bloc, size) ) == nullptr) {
        
        this_bloc = extend_heap(base_bloc, size);
    }
    return this_bloc->data;
}
// Create array of N members of size S
void * calloc(size_t num_members, size_t size){
    return malloc( num_members * size );
}
// Free malloc()'d object. If it isn't from malloc(), it'll just not do anything. Make sure to free things manually in that case!
void free( void * ptr) {
    if (ptr == nullptr) {}
    m_bloc * this_bloc;
    if ((this_bloc = get_bloc(ptr)) == nullptr) { return; }
    this_bloc->isFree = true;
    this_bloc->data = nullptr;
}




// Copy N bytes from src to dest
void memcpy(void* dest, void* src, size_t n) {
    char *chsrc = (char *)src;
    char *chdest = (char *)dest;
    while ((*chdest++ = *chsrc++) < n);
}
// Set n bytes of src to value v (char)
void memset(void* src, char v, size_t n) {
    char *chptr = (char *)src;
    while ((*chptr++ = v) < n);
}
// Set n amounts of src to v  (32 bit long)
void memset(void* src, int v, size_t n) {
    int *chptr = (int *)src;
    while ((*chptr++ = v) < n);
}


#endif // clearos_os_cstd_alloc_h__
