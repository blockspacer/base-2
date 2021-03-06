#ifndef KUDU_FS_BLOCK_MANAGER_UTIL_H
#define KUDU_FS_BLOCK_MANAGER_UTIL_H

#include <string>
#include <vector>

#include "base/core/gscoped_ptr.h"
#include "base/util/path_util.h"
#include "base/util/status.h"

namespace base {

class Env;
class FileLock;
class PathInstanceMetadataPB;

namespace fs {

// Reads and writes block manager instance metadata files.
//
// Thread-unsafe; access to this object must be externally synchronized.
class PathInstanceMetadataFile {
 public:
  // 'env' must remain valid for the lifetime of this class.
  PathInstanceMetadataFile(Env* env, std::string block_manager_type,
                           std::string filename);

  ~PathInstanceMetadataFile();

  // Creates, writes, synchronizes, and closes a new instance metadata file.
  //
  // 'uuid' is this instance's UUID, and 'all_uuids' is all of the UUIDs in
  // this instance's path set.
  Status Create(const std::string& uuid,
                const std::vector<std::string>& all_uuids);

  // Opens, reads, verifies, and closes an existing instance metadata file.
  //
  // On success, 'metadata_' is overwritten with the contents of the file.
  Status LoadFromDisk();

  // Locks the instance metadata file, which must exist on-disk. Returns an
  // error if it's already locked. The lock is released when Unlock() is
  // called, when this object is destroyed, or when the process exits.
  //
  // Note: the lock is also released if any fd of the instance metadata file
  // in this process is closed. Thus, it is an error to call Create() or
  // LoadFromDisk() on a locked file.
  Status Lock();

  // Unlocks the instance metadata file. Must have been locked to begin with.
  Status Unlock();

  void SetMetadataForTests(gscoped_ptr<PathInstanceMetadataPB> metadata) {
    metadata_ = std::move(metadata);
  }

  std::string path() const { return DirName(filename_); }
  PathInstanceMetadataPB* const metadata() const { return metadata_.get(); }

  // Check the integrity of the provided instances' path sets.
  static Status CheckIntegrity(const std::vector<PathInstanceMetadataFile*>& instances);

 private:
  Env* env_;
  const std::string block_manager_type_;
  const std::string filename_;
  gscoped_ptr<PathInstanceMetadataPB> metadata_;
  gscoped_ptr<FileLock> lock_;
};

} // namespace fs
} // namespace base
#endif
