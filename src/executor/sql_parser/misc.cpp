#include "tokenizer.h"

namespace JfEngine {

Expected<TTableInputPtr> IoperatorCommand::MakeWorker() {
    return MakeError<EError::UnsupportedErr>();
}

} // namespace JfEngine
