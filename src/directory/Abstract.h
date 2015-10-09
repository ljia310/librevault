/* Copyright (C) 2015 Alexander Shishenko <GamePad64@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../pch.h"
#pragma once
#include "Meta.h"

namespace librevault {

class Session;
class Exchanger;

class AbstractDirectory {
public:
	struct SignedMeta {
		blob meta;
		blob signature;
	};

	AbstractDirectory(Session& session, Exchanger& exchanger);
	virtual ~AbstractDirectory();

	// AbstractDirectory properties
	virtual std::string name() const = 0;
	virtual std::vector<Meta::PathRevision> get_meta_list() = 0;

	// AbstractDirectory actions
	virtual void post_revision(std::shared_ptr<AbstractDirectory> origin, const Meta::PathRevision& revision) = 0;
	virtual void request_meta(std::shared_ptr<AbstractDirectory> origin, const blob& path_id) = 0;
	virtual void post_meta(std::shared_ptr<AbstractDirectory> origin, const SignedMeta& smeta) = 0;
	virtual void request_block(std::shared_ptr<AbstractDirectory> origin, const blob& block_id) = 0;	// TODO: Tie this to BitTorrent behavior. Request every block in small 16kb (or more) chunks
	virtual void post_block(std::shared_ptr<AbstractDirectory> origin, const blob& block_id, const blob& block) = 0;	// TODO: Tie this to BitTorrent behavior. Send every block in small 16kb (or more) chunks

	// Other functions
	std::string path_id_readable(const blob& path_id) const;
	std::string block_id_readable(const blob& block_id) const;
	std::string log_tag() const {return std::string("[") + name() + "] ";}

protected:
	Session& session_;
	Exchanger& exchanger_;

	logger_ptr log_;
};

} /* namespace librevault */