#ifndef KUDU_CONSENSUS_OPID_UTIL_H_
#define KUDU_CONSENSUS_OPID_UTIL_H_

#include <stdint.h>

#include <iosfwd>
#include <string>
#include <utility>

namespace base {
namespace consensus {

class ConsensusRequestPB;
class OpId;

// Minimum possible term.
extern const int64_t kMinimumTerm;

// Minimum possible log index.
extern const int64_t kMinimumOpIdIndex;

// Log index that is lower than the minimum index (and so will never occur).
extern const int64_t kInvalidOpIdIndex;

// Returns true iff left == right.
bool OpIdEquals(const OpId& left, const OpId& right);

// Returns true iff left < right.
bool OpIdLessThan(const OpId& left, const OpId& right);

// Returns true iff left > right.
bool OpIdBiggerThan(const OpId& left, const OpId& right);

// Copies to_compare into target under the following conditions:
// - If to_compare is initialized and target is not.
// - If they are both initialized and to_compare is less than target.
// Otherwise, does nothing.
// If to_compare is copied into target, returns true, else false.
bool CopyIfOpIdLessThan(const OpId& to_compare, OpId* target);

// Return -1 if left < right,
//         0 if equal,
//         1 if left > right.
int OpIdCompare(const OpId& left, const OpId& right);

// OpId hash functor. Suitable for use with std::unordered_map.
struct OpIdHashFunctor {
  size_t operator() (const OpId& id) const;
};

// OpId equals functor. Suitable for use with std::unordered_map.
struct OpIdEqualsFunctor {
  bool operator() (const OpId& left, const OpId& right) const;
};

// OpId less than functor for pointers.. Suitable for use with std::sort and std::map.
struct OpIdLessThanPtrFunctor {
  // Returns true iff left < right.
  bool operator() (const OpId* left, const OpId* right) const;
};

// Sorts op id's by index only, disregarding the term.
struct OpIdIndexLessThanPtrFunctor {
  // Returns true iff left.index() < right.index().
  bool operator() (const OpId* left, const OpId* right) const;
};

// OpId compare() functor. Suitable for use with std::sort and std::map.
struct OpIdCompareFunctor {
  // Returns true iff left < right.
  bool operator() (const OpId& left, const OpId& right) const;
};

// OpId comparison functor that returns true iff left > right. Suitable for use
// td::sort and std::map to sort keys in increasing order.]
struct OpIdBiggerThanFunctor {
  bool operator() (const OpId& left, const OpId& right) const;
};

std::ostream& operator<<(std::ostream& os, const consensus::OpId& op_id);

// Return the minimum possible OpId.
OpId MinimumOpId();

// Return the maximum possible OpId.
OpId MaximumOpId();

std::string OpIdToString(const OpId& id);

std::string OpsRangeString(const ConsensusRequestPB& req);

OpId MakeOpId(int term, int index);

}  // namespace consensus
}  // namespace base

#endif /* KUDU_CONSENSUS_OPID_UTIL_H_ */
