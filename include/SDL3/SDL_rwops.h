/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_rwops.h
 *
 *  This file provides a general interface for SDL to read and write
 *  data streams.  It can easily be extended to files, memory, etc.
 */

#ifndef SDL_rwops_h_
#define SDL_rwops_h_

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_properties.h>

#include <SDL3/SDL_begin_code.h>
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* RWops status, set by a read or write operation */
typedef enum SDL_RWopsStatus
{
    SDL_RWOPS_STATUS_READY,     /**< Everything is ready */
    SDL_RWOPS_STATUS_ERROR,     /**< Read or write I/O error */
    SDL_RWOPS_STATUS_EOF,       /**< End of file */
    SDL_RWOPS_STATUS_NOT_READY, /**< Non blocking I/O, not ready */
    SDL_RWOPS_STATUS_READONLY,  /**< Tried to write a read-only buffer */
    SDL_RWOPS_STATUS_WRITEONLY  /**< Tried to read a write-only buffer */
} SDL_RWopsStatus;

typedef struct SDL_RWopsInterface
{
    /**
     *  Return the number of bytes in this rwops
     *
     *  \return the total size of the data stream, or -1 on error.
     */
    Sint64 (SDLCALL *size)(void *userdata);

    /**
     *  Seek to \c offset relative to \c whence, one of stdio's whence values:
     *  SDL_RW_SEEK_SET, SDL_RW_SEEK_CUR, SDL_RW_SEEK_END
     *
     *  \return the final offset in the data stream, or -1 on error.
     */
    Sint64 (SDLCALL *seek)(void *userdata, Sint64 offset, int whence);

    /**
     *  Read up to \c size bytes from the data stream to the area pointed
     *  at by \c ptr.
     *
     *  \return the number of bytes read
     */
    size_t (SDLCALL *read)(void *userdata, void *ptr, size_t size);

    /**
     *  Write exactly \c size bytes from the area pointed at by \c ptr
     *  to data stream.
     *
     *  \return the number of bytes written
     */
    size_t (SDLCALL *write)(void *userdata, const void *ptr, size_t size);

    /**
     *  Close and free any allocated resources.
     *
     *  The RWops is still destroyed even if this fails, so clean up anything
     *  even if flushing to disk returns an error.
     *
     *  \return 0 if successful or -1 on write error when flushing data.
     */
    int (SDLCALL *close)(void *userdata);
} SDL_RWopsInterface;


/**
 * This is the read/write operation structure -- opaque, as of SDL3!
 */
typedef struct SDL_RWops SDL_RWops;


/**
 *  \name RWFrom functions
 *
 *  Functions to create SDL_RWops structures from various data streams.
 */
/* @{ */

/**
 * Use this function to create a new SDL_RWops structure for reading from
 * and/or writing to a named file.
 *
 * The `mode` string is treated roughly the same as in a call to the C
 * library's fopen(), even if SDL doesn't happen to use fopen() behind the
 * scenes.
 *
 * Available `mode` strings:
 *
 * - "r": Open a file for reading. The file must exist.
 * - "w": Create an empty file for writing. If a file with the same name
 *   already exists its content is erased and the file is treated as a new
 *   empty file.
 * - "a": Append to a file. Writing operations append data at the end of the
 *   file. The file is created if it does not exist.
 * - "r+": Open a file for update both reading and writing. The file must
 *   exist.
 * - "w+": Create an empty file for both reading and writing. If a file with
 *   the same name already exists its content is erased and the file is
 *   treated as a new empty file.
 * - "a+": Open a file for reading and appending. All writing operations are
 *   performed at the end of the file, protecting the previous content to be
 *   overwritten. You can reposition (fseek, rewind) the internal pointer to
 *   anywhere in the file for reading, but writing operations will move it
 *   back to the end of file. The file is created if it does not exist.
 *
 * **NOTE**: In order to open a file as a binary file, a "b" character has to
 * be included in the `mode` string. This additional "b" character can either
 * be appended at the end of the string (thus making the following compound
 * modes: "rb", "wb", "ab", "r+b", "w+b", "a+b") or be inserted between the
 * letter and the "+" sign for the mixed modes ("rb+", "wb+", "ab+").
 * Additional characters may follow the sequence, although they should have no
 * effect. For example, "t" is sometimes appended to make explicit the file is
 * a text file.
 *
 * This function supports Unicode filenames, but they must be encoded in UTF-8
 * format, regardless of the underlying operating system.
 *
 * As a fallback, SDL_RWFromFile() will transparently open a matching filename
 * in an Android app's `assets`.
 *
 * Destroying the SDL_RWops will close the file handle SDL is holding internally.
 *
 * The following properties may be set at creation time by SDL:
 *
 * - `SDL_PROP_RWOPS_WINDOWS_HANDLE_POINTER`: a pointer, that can be cast
 *   to a win32 `HANDLE`, that this RWops is using to access the filesystem.
 *   If the program isn't running on Windows, or SDL used some other method
 *   to access the filesystem, this property will not be set.
 * - `SDL_PROP_RWOPS_STDIO_HANDLE_POINTER`: a pointer, that can be cast
 *   to a stdio `FILE *`, that this RWops is using to access the filesystem.
 *   If SDL used some other method to access the filesystem, this property
 *   will not be set. PLEASE NOTE that if SDL is using a different C runtime
 *   than your app, trying to use this pointer will almost certainly result
 *   in a crash! This is mostly a problem on Windows; make sure you build SDL
 *   and your app with the same compiler and settings to avoid it.
 * - `SDL_PROP_RWOPS_ANDROID_AASSET_POINTER`: a pointer, that can be cast
 *   to an Android NDK `AAsset *`, that this RWops is using to access the
 *   filesystem. If SDL used some other method to access the filesystem, this
 *   property will not be set.
 *
 * \param file a UTF-8 string representing the filename to open
 * \param mode an ASCII string representing the mode to be used for opening
 *             the file.
 * \returns a pointer to the SDL_RWops structure that is created, or NULL on
 *          failure; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_TellRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC SDL_RWops *SDLCALL SDL_RWFromFile(const char *file, const char *mode);

#define SDL_PROP_RWOPS_WINDOWS_HANDLE_POINTER "SDL.rwops.windows.handle"
#define SDL_PROP_RWOPS_STDIO_HANDLE_POINTER "SDL.rwops.stdio.handle"
#define SDL_PROP_RWOPS_ANDROID_AASSET_POINTER "SDL.rwops.android.aasset"

/**
 * Use this function to prepare a read-write memory buffer for use with
 * SDL_RWops.
 *
 * This function sets up an SDL_RWops struct based on a memory area of a
 * certain size, for both read and write access.
 *
 * This memory buffer is not copied by the RWops; the pointer you provide must
 * remain valid until you close the stream. Closing the stream will not free
 * the original buffer.
 *
 * If you need to make sure the RWops never writes to the memory buffer, you
 * should use SDL_RWFromConstMem() with a read-only buffer of memory instead.
 *
 * \param mem a pointer to a buffer to feed an SDL_RWops stream
 * \param size the buffer size, in bytes
 * \returns a pointer to a new SDL_RWops structure, or NULL if it fails; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_TellRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC SDL_RWops *SDLCALL SDL_RWFromMem(void *mem, size_t size);

/**
 * Use this function to prepare a read-only memory buffer for use with RWops.
 *
 * This function sets up an SDL_RWops struct based on a memory area of a
 * certain size. It assumes the memory area is not writable.
 *
 * Attempting to write to this RWops stream will report an error without
 * writing to the memory buffer.
 *
 * This memory buffer is not copied by the RWops; the pointer you provide must
 * remain valid until you close the stream. Closing the stream will not free
 * the original buffer.
 *
 * If you need to write to a memory buffer, you should use SDL_RWFromMem()
 * with a writable buffer of memory instead.
 *
 * \param mem a pointer to a read-only buffer to feed an SDL_RWops stream
 * \param size the buffer size, in bytes
 * \returns a pointer to a new SDL_RWops structure, or NULL if it fails; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_TellRW
 */
extern DECLSPEC SDL_RWops *SDLCALL SDL_RWFromConstMem(const void *mem, size_t size);

/* @} *//* RWFrom functions */


/**
 * Create a custom SDL_RWops.
 *
 * Applications do not need to use this function unless they are providing
 * their own SDL_RWops implementation. If you just need an SDL_RWops to
 * read/write a common data source, you should use the built-in
 * implementations in SDL, like SDL_RWFromFile() or SDL_RWFromMem(), etc.
 *
 * You must free the returned pointer with SDL_CloseRW().
 *
 * \returns a pointer to the allocated memory on success, or NULL on failure;
 *          call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_CloseRW
 */
extern DECLSPEC SDL_RWops *SDLCALL SDL_OpenRW(const SDL_RWopsInterface *iface, void *userdata);

/**
 * Close and free an allocated SDL_RWops structure.
 *
 * SDL_CloseRW() closes and cleans up the SDL_RWops stream. It releases any
 * resources used by the stream and frees the SDL_RWops itself with
 * SDL_CloseRW(). This returns 0 on success, or -1 if the stream failed to
 * flush to its output (e.g. to disk).
 *
 * Note that if this fails to flush the stream to disk, this function reports
 * an error, but the SDL_RWops is still invalid once this function returns.
 *
 * \param context SDL_RWops structure to close
 * \returns 0 on success or a negative error code on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC int SDLCALL SDL_CloseRW(SDL_RWops *context);

/**
 * Get the properties associated with an SDL_RWops.
 *
 * \param context a pointer to an SDL_RWops structure
 * \returns a valid property ID on success or 0 on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_GetProperty
 * \sa SDL_SetProperty
 */
extern DECLSPEC SDL_PropertiesID SDLCALL SDL_GetRWProperties(SDL_RWops *context);

#define SDL_RW_SEEK_SET 0       /**< Seek from the beginning of data */
#define SDL_RW_SEEK_CUR 1       /**< Seek relative to current read point */
#define SDL_RW_SEEK_END 2       /**< Seek relative to the end of data */

/**
 * Query the stream status of a RWops.
 *
 * This information can be useful to decide if a short read or write was
 * due to an error, an EOF, or a non-blocking operation that isn't yet
 * ready to complete.
 *
 * A RWops's status is only expected to change after a SDL_ReadRW or
 * SDL_WriteRW call; don't expect it to change if you just call this
 * query function in a tight loop.
 *
 * \param context the SDL_RWops to query.
 * \returns an SDL_RWopsStatus enum with the current state.
 *
 * \threadsafety This function should not be called at the same time that
 *               another thread is operating on the same SDL_RWops.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_RWopsStatus SDLCALL SDL_GetRWStatus(SDL_RWops *context);

/**
 * Use this function to get the size of the data stream in an SDL_RWops.
 *
 * \param context the SDL_RWops to get the size of the data stream from
 * \returns the size of the data stream in the SDL_RWops on success or a
 *          negative error code on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC Sint64 SDLCALL SDL_SizeRW(SDL_RWops *context);

/**
 * Seek within an SDL_RWops data stream.
 *
 * This function seeks to byte `offset`, relative to `whence`.
 *
 * `whence` may be any of the following values:
 *
 * - `SDL_RW_SEEK_SET`: seek from the beginning of data
 * - `SDL_RW_SEEK_CUR`: seek relative to current read point
 * - `SDL_RW_SEEK_END`: seek relative to the end of data
 *
 * If this stream can not seek, it will return -1.
 *
 * \param context a pointer to an SDL_RWops structure
 * \param offset an offset in bytes, relative to **whence** location; can be
 *               negative
 * \param whence any of `SDL_RW_SEEK_SET`, `SDL_RW_SEEK_CUR`,
 *               `SDL_RW_SEEK_END`
 * \returns the final offset in the data stream after the seek or a negative
 *          error code on failure; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_TellRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC Sint64 SDLCALL SDL_SeekRW(SDL_RWops *context, Sint64 offset, int whence);

/**
 * Determine the current read/write offset in an SDL_RWops data stream.
 *
 * SDL_TellRW is actually a wrapper function that calls the SDL_RWops's `seek`
 * method, with an offset of 0 bytes from `SDL_RW_SEEK_CUR`, to simplify
 * application development.
 *
 * \param context an SDL_RWops data stream object from which to get the
 *                current offset
 * \returns the current offset in the stream, or -1 if the information can not
 *          be determined.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC Sint64 SDLCALL SDL_TellRW(SDL_RWops *context);

/**
 * Read from a data source.
 *
 * This function reads up `size` bytes from the data source to the area
 * pointed at by `ptr`. This function may read less bytes than requested. It
 * will return zero when the data stream is completely read, or -1 on error.
 * For streams that support non-blocking operation, if nothing was read
 * because it would require blocking, this function returns -2 to distinguish
 * that this is not an error or end-of-file, and the caller can try again
 * later.
 *
 * SDL_ReadRW() is actually a function wrapper that calls the SDL_RWops's
 * `read` method appropriately, to simplify application development.
 *
 * \param context a pointer to an SDL_RWops structure
 * \param ptr a pointer to a buffer to read data into
 * \param size the number of bytes to read from the data source.
 * \returns the number of bytes read, or 0 on end of file or other error.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_SeekRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC size_t SDLCALL SDL_ReadRW(SDL_RWops *context, void *ptr, size_t size);

/**
 * Write to an SDL_RWops data stream.
 *
 * This function writes exactly `size` bytes from the area pointed at by `ptr`
 * to the stream. If this fails for any reason, it'll return less than `size`
 * to demonstrate how far the write progressed. On success, it returns `num`.
 *
 * On error, this function still attempts to write as much as possible, so it
 * might return a positive value less than the requested write size. If the
 * function failed to write anything and there was an actual error, it will
 * return -1. For streams that support non-blocking operation, if nothing was
 * written because it would require blocking, this function returns -2 to
 * distinguish that this is not an error and the caller can try again later.
 *
 * SDL_WriteRW is actually a function wrapper that calls the SDL_RWops's
 * `write` method appropriately, to simplify application development.
 *
 * It is an error to specify a negative `size`, but this parameter is signed
 * so you definitely cannot overflow the return value on a successful run with
 * enormous amounts of data.
 *
 * \param context a pointer to an SDL_RWops structure
 * \param ptr a pointer to a buffer containing data to write
 * \param size the number of bytes to write
 * \returns the number of bytes written, which will be less than `num` on
 *          error; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_RWprint
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 */
extern DECLSPEC size_t SDLCALL SDL_WriteRW(SDL_RWops *context, const void *ptr, size_t size);

/**
 * Print to an SDL_RWops data stream.
 *
 * This function does formatted printing to the stream.
 *
 * \param context a pointer to an SDL_RWops structure
 * \param fmt a printf() style format string
 * \param ... additional parameters matching % tokens in the `fmt` string, if
 *            any
 * \returns the number of bytes written, or 0 on error; call SDL_GetError()
 *          for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC size_t SDLCALL SDL_RWprintf(SDL_RWops *context, SDL_PRINTF_FORMAT_STRING const char *fmt, ...)  SDL_PRINTF_VARARG_FUNC(2);

/**
 * Print to an SDL_RWops data stream.
 *
 * This function does formatted printing to the stream.
 *
 * \param context a pointer to an SDL_RWops structure
 * \param fmt a printf() style format string
 * \param ap a variable argument list
 * \returns the number of bytes written, or 0 on error; call SDL_GetError()
 *          for more information.
 *
 * \since This function is available since SDL 3.0.0.
 *
 * \sa SDL_RWFromConstMem
 * \sa SDL_RWFromFile
 * \sa SDL_RWFromMem
 * \sa SDL_ReadRW
 * \sa SDL_SeekRW
 * \sa SDL_WriteRW
 */
extern DECLSPEC size_t SDLCALL SDL_RWvprintf(SDL_RWops *context, SDL_PRINTF_FORMAT_STRING const char *fmt, va_list ap) SDL_PRINTF_VARARG_FUNCV(2);

/**
 * Load all the data from an SDL data stream.
 *
 * The data is allocated with a zero byte at the end (null terminated) for
 * convenience. This extra byte is not included in the value reported via
 * `datasize`.
 *
 * The data should be freed with SDL_free().
 *
 * \param src the SDL_RWops to read all available data from
 * \param datasize if not NULL, will store the number of bytes read
 * \param freesrc if SDL_TRUE, calls SDL_CloseRW() on `src` before returning,
 *                even in the case of an error
 * \returns the data, or NULL if there was an error.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC void *SDLCALL SDL_LoadFile_RW(SDL_RWops *src, size_t *datasize, SDL_bool freesrc);

/**
 * Load all the data from a file path.
 *
 * The data is allocated with a zero byte at the end (null terminated) for
 * convenience. This extra byte is not included in the value reported via
 * `datasize`.
 *
 * The data should be freed with SDL_free().
 *
 * \param file the path to read all available data from
 * \param datasize if not NULL, will store the number of bytes read
 * \returns the data, or NULL if there was an error.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC void *SDLCALL SDL_LoadFile(const char *file, size_t *datasize);

/**
 *  \name Read endian functions
 *
 *  Read an item of the specified endianness and return in native format.
 */
/* @{ */

/**
 * Use this function to read a byte from an SDL_RWops.
 *
 * \param src the SDL_RWops to read from
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on success or SDL_FALSE on failure; call SDL_GetError()
 *          for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU8(SDL_RWops *src, Uint8 *value);

/**
 * Use this function to read 16 bits of little-endian data from an SDL_RWops
 * and return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU16LE(SDL_RWops *src, Uint16 *value);

/**
 * Use this function to read 16 bits of little-endian data from an SDL_RWops
 * and return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadS16LE(SDL_RWops *src, Sint16 *value);

/**
 * Use this function to read 16 bits of big-endian data from an SDL_RWops and
 * return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU16BE(SDL_RWops *src, Uint16 *value);

/**
 * Use this function to read 16 bits of big-endian data from an SDL_RWops and
 * return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadS16BE(SDL_RWops *src, Sint16 *value);

/**
 * Use this function to read 32 bits of little-endian data from an SDL_RWops
 * and return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU32LE(SDL_RWops *src, Uint32 *value);

/**
 * Use this function to read 32 bits of little-endian data from an SDL_RWops
 * and return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadS32LE(SDL_RWops *src, Sint32 *value);

/**
 * Use this function to read 32 bits of big-endian data from an SDL_RWops and
 * return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU32BE(SDL_RWops *src, Uint32 *value);

/**
 * Use this function to read 32 bits of big-endian data from an SDL_RWops and
 * return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadS32BE(SDL_RWops *src, Sint32 *value);

/**
 * Use this function to read 64 bits of little-endian data from an SDL_RWops
 * and return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU64LE(SDL_RWops *src, Uint64 *value);

/**
 * Use this function to read 64 bits of little-endian data from an SDL_RWops
 * and return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadS64LE(SDL_RWops *src, Sint64 *value);

/**
 * Use this function to read 64 bits of big-endian data from an SDL_RWops and
 * return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadU64BE(SDL_RWops *src, Uint64 *value);

/**
 * Use this function to read 64 bits of big-endian data from an SDL_RWops and
 * return in native format.
 *
 * SDL byteswaps the data only if necessary, so the data returned will be in
 * the native byte order.
 *
 * \param src the stream from which to read data
 * \param value a pointer filled in with the data read
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_ReadS64BE(SDL_RWops *src, Sint64 *value);
/* @} *//* Read endian functions */

/**
 *  \name Write endian functions
 *
 *  Write an item of native format to the specified endianness.
 */
/* @{ */

/**
 * Use this function to write a byte to an SDL_RWops.
 *
 * \param dst the SDL_RWops to write to
 * \param value the byte value to write
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU8(SDL_RWops *dst, Uint8 value);

/**
 * Use this function to write 16 bits in native format to an SDL_RWops as
 * little-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in little-endian
 * format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU16LE(SDL_RWops *dst, Uint16 value);

/**
 * Use this function to write 16 bits in native format to an SDL_RWops as
 * little-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in little-endian
 * format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteS16LE(SDL_RWops *dst, Sint16 value);

/**
 * Use this function to write 16 bits in native format to an SDL_RWops as
 * big-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in big-endian format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU16BE(SDL_RWops *dst, Uint16 value);

/**
 * Use this function to write 16 bits in native format to an SDL_RWops as
 * big-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in big-endian format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteS16BE(SDL_RWops *dst, Sint16 value);

/**
 * Use this function to write 32 bits in native format to an SDL_RWops as
 * little-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in little-endian
 * format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU32LE(SDL_RWops *dst, Uint32 value);

/**
 * Use this function to write 32 bits in native format to an SDL_RWops as
 * little-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in little-endian
 * format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteS32LE(SDL_RWops *dst, Sint32 value);

/**
 * Use this function to write 32 bits in native format to an SDL_RWops as
 * big-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in big-endian format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU32BE(SDL_RWops *dst, Uint32 value);

/**
 * Use this function to write 32 bits in native format to an SDL_RWops as
 * big-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in big-endian format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteS32BE(SDL_RWops *dst, Sint32 value);

/**
 * Use this function to write 64 bits in native format to an SDL_RWops as
 * little-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in little-endian
 * format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU64LE(SDL_RWops *dst, Uint64 value);

/**
 * Use this function to write 64 bits in native format to an SDL_RWops as
 * little-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in little-endian
 * format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteS64LE(SDL_RWops *dst, Sint64 value);

/**
 * Use this function to write 64 bits in native format to an SDL_RWops as
 * big-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in big-endian format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteU64BE(SDL_RWops *dst, Uint64 value);

/**
 * Use this function to write 64 bits in native format to an SDL_RWops as
 * big-endian data.
 *
 * SDL byteswaps the data only if necessary, so the application always
 * specifies native format, and the data written will be in big-endian format.
 *
 * \param dst the stream to which data will be written
 * \param value the data to be written, in native format
 * \returns SDL_TRUE on successful write, SDL_FALSE on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.0.0.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_WriteS64BE(SDL_RWops *dst, Sint64 value);

/* @} *//* Write endian functions */

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include <SDL3/SDL_close_code.h>

#endif /* SDL_rwops_h_ */
