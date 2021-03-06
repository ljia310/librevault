/* Copyright (C) 2016 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#include "MetaStorage.h"
#include "control/FolderParams.h"
#include "folder/PathNormalizer.h"
#include "Index.h"
#include "Indexer.h"
#include "AutoIndexer.h"

namespace librevault {

MetaStorage::MetaStorage(const FolderParams& params, IgnoreList& ignore_list, PathNormalizer& path_normalizer, io_service& ios) {
	index = std::make_unique<Index>(params);
	if(params.secret.get_type() <= Secret::Type::ReadWrite){
		indexer_ = std::make_unique<Indexer>(params, *index, ignore_list, path_normalizer, ios);
		auto_indexer_ = std::make_unique<AutoIndexer>(params, *index, *indexer_, ignore_list, path_normalizer, ios);
	}
};
MetaStorage::~MetaStorage() {}

bool MetaStorage::is_indexing() const {
	return (indexer_ && indexer_->is_indexing());
}

void MetaStorage::prepare_assemble(const std::string relpath, Meta::Type type, bool with_removal) {
	if(auto_indexer_) auto_indexer_->prepare_assemble(relpath, type, with_removal);
}

} /* namespace librevault */
