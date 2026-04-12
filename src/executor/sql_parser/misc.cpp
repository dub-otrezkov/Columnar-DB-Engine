#include "tokenizer.h"

namespace JfEngine {

Expected<ITableInput> IoperatorCommand::MakeWorker() {
    return MakeError<EError::UnsupportedErr>();
}

} // namespace JfEngine
