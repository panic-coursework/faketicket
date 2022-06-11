#ifndef TICKET_LIB_FILE_INTERNAL_UNMANAGED_OBJECT_H_
#define TICKET_LIB_FILE_INTERNAL_UNMANAGED_OBJECT_H_

#include "file/file.h"

namespace ticket::file::internal {

template <typename T, typename Meta = Unit, size_t szChunk = kDefaultSzChunk>
class UnmanagedObject {
 private:
  using File_ = File<Meta, szChunk>;
  File_ *file_;
  size_t id_ = -1;
  UnmanagedObject (File_ &file, size_t id) : file_(&file), id_(id) {}
  static size_t getSize_ () { return offsetof(T, _end) - offsetof(T, _start); }
  static size_t getOffset_ () { return offsetof(T, _start); }
 public:
  UnmanagedObject () = delete;
  UnmanagedObject (File_ &file) : file_(&file) {}
  virtual ~UnmanagedObject () = default;

  size_t id () { return id_; }

  static T get (File_ &file, size_t id) {
    char buf[sizeof(T)];
    file.get(buf + getOffset_(), id, getSize_());
    UnmanagedObject &result = *reinterpret_cast<UnmanagedObject *>(buf);
    result.file_ = &file;
    result.id_ = id;
    return *reinterpret_cast<T *>(buf);
  }
  void save () {
    if (id_ != -1) throw Exception("Already saved");
    id_ = file_->push(reinterpret_cast<char *>(this) + getOffset_(), getSize_());
  }
  void update () {
    if (id_ == -1) throw Exception("Not saved");
    file_->set(reinterpret_cast<char *>(this) + getOffset_(), id_, getSize_());
  }
  void destroy () {
    if (id_ == -1) throw Exception("Not saved");
    file_->remove(id_);
    id_ = -1;
  }
};

} // namespace ticket::file::internal

#endif // TICKET_LIB_FILE_INTERNAL_UNMANAGED_OBJECT_H_
