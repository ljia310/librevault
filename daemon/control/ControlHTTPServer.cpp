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
#include "ControlHTTPServer.h"
#include "Client.h"
#include "control/Config.h"
#include "util/log.h"
#include <boost/algorithm/string/predicate.hpp>

namespace librevault {

ControlHTTPServer::ControlHTTPServer(Client& client, ControlServer& cs, ControlServer::server& server, io_service& ios) :
		client_(client), cs_(cs), server_(server), ios_(ios) {
	handlers_.push_back(std::make_pair(std::regex(R"(^\/v1\/status\/?$)"), [this](ControlServer::server::connection_ptr conn, std::smatch matched){handle_status(conn, matched);}));
	handlers_.push_back(std::make_pair(std::regex(R"(^\/v1\/version\/?$)"), [this](ControlServer::server::connection_ptr conn, std::smatch matched){handle_version(conn, matched);}));
	handlers_.push_back(std::make_pair(std::regex(R"(^\/v1\/restart\/?$)"), [this](ControlServer::server::connection_ptr conn, std::smatch matched){handle_restart(conn, matched);}));
	handlers_.push_back(std::make_pair(std::regex(R"(^\/v1\/shutdown\/?$)"), [this](ControlServer::server::connection_ptr conn, std::smatch matched){handle_shutdown(conn, matched);}));
	handlers_.push_back(std::make_pair(std::regex(R"(^\/v1\/globals(?:\/(\w+?))?\/?$)"), [this](ControlServer::server::connection_ptr conn, std::smatch matched){handle_globals(conn, matched);}));
}

ControlHTTPServer::~ControlHTTPServer() {}

void ControlHTTPServer::on_http(websocketpp::connection_hdl hdl) {
	LOGFUNC();

	auto connection_ptr = server_.get_con_from_hdl(hdl);

	// CORS
	//if(!cs_.check_origin(connection_ptr)) {
	//	connection_ptr->set_status(websocketpp::http::status_code::forbidden);
	//	return;
	//}

	// URI handlers
	const std::string& uri = connection_ptr->get_request().get_uri();
	std::smatch uri_match;
	for(auto& handler : handlers_) {
		if(std::regex_match(uri, uri_match, handler.first)) {
			handler.second(connection_ptr, uri_match);
			break;
		}
	}
	if(uri_match.empty()) {
		connection_ptr->set_status(websocketpp::http::status_code::not_implemented);
	}
}

void ControlHTTPServer::handle_status(ControlServer::server::connection_ptr conn, std::smatch matched) {
	conn->set_status(websocketpp::http::status_code::ok);
	conn->set_body(cs_.make_control_json());
}

void ControlHTTPServer::handle_version(ControlServer::server::connection_ptr conn, std::smatch matched) {
	conn->set_status(websocketpp::http::status_code::ok);
	conn->append_header("Content-Type", "text/plain");
	conn->set_body(Version::current().version_string());
}

void ControlHTTPServer::handle_restart(ControlServer::server::connection_ptr conn, std::smatch matched) {
	conn->set_status(websocketpp::http::status_code::ok);
	ios_.post([this]{cs_.restart_signal();});
}

void ControlHTTPServer::handle_shutdown(ControlServer::server::connection_ptr conn, std::smatch matched) {
	conn->set_status(websocketpp::http::status_code::ok);
	ios_.post([this]{cs_.shutdown_signal();});
}

void ControlHTTPServer::handle_globals(ControlServer::server::connection_ptr conn, std::smatch matched) {
	if(conn->get_request().get_method() == "GET" && !matched[1].matched) {
		conn->set_status(websocketpp::http::status_code::ok);
		conn->append_header("Content-Type", "text/x-json");
		conn->set_body(Json::FastWriter().write(Config::get()->globals()));
	}else if(conn->get_request().get_method() == "PUT" && !matched[1].matched) {
		conn->set_status(websocketpp::http::status_code::ok);

		Json::Value new_value;
		Json::Reader().parse(conn->get_request_body(), new_value);

		Config::get()->set_globals(new_value);
	}else if(conn->get_request().get_method() == "GET" && matched[1].matched){
		conn->set_status(websocketpp::http::status_code::ok);
		conn->append_header("Content-Type", "text/x-json");
		conn->set_body(Json::FastWriter().write(Config::get()->global_get(matched[1].str())));
	}else if(conn->get_request().get_method() == "PUT" && matched[1].matched){
		conn->set_status(websocketpp::http::status_code::ok);

		Json::Value new_value;
		Json::Reader().parse(conn->get_request_body(), new_value);

		Config::get()->global_set(matched[1].str(), new_value);
	}else if(conn->get_request().get_method() == "DELETE" && matched[1].matched){
		conn->set_status(websocketpp::http::status_code::ok);
		Config::get()->global_unset(matched[1].str());
	}
}

} /* namespace librevault */
