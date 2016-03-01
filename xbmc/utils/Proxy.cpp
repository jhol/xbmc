/*
 *      Copyright (C) 2016 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <sstream>

#include "Archive.h"

#include "Proxy.h"

using std::string;

const uint16_t DefaultPorts[] = {
  3128,
  1080,
  1080,
  1080,
  1080
};

const char* ProtocolNames[] = {
  "http",
  "socks4",
  "socks4a",
  "socks5",
  "socks5remote"
};

CProxy::CProxy()
  : m_type(ProxyHttp)
  , m_port(DefaultPorts[ProxyHttp])
{
}

CProxy::CProxy(const string &url)
{
  SetUrl(url);
}

CProxy::CProxy(Type type, const string &hostPortUserPassword)
  : m_type(type)
{
  SetHostWithPortUserPassword(hostPortUserPassword);
}

CProxy::CProxy(Type type,
  const string& host,
  uint16_t port,
  const string& user,
  const string& password)
  : m_type(type)
  , m_host(host)
  , m_port(port)
  , m_user(user)
  , m_password(password)
{
}

CProxy::~CProxy()
{
}

string CProxy::GetUrl() const
{
  return string(ProtocolNames[m_type]) + "://" + GetHostWithPortUserPassword();
}

void CProxy::SetUrl(const string &url)
{
  const size_t count = sizeof(ProtocolNames) / sizeof(ProtocolNames[0]);
  for (size_t i = 0; i != count; i++) {
    const string protocol_name = string(ProtocolNames[i]) + "://";
    if (url.compare(0, protocol_name.size(), protocol_name) == 0) {
      m_type = static_cast<CProxy::Type>(i);
      SetHostWithPortUserPassword(url.substr(protocol_name.size()));
      break;
    }
  }
}

string CProxy::GetHostWithPortUserPassword() const
{
  const string userPassword = GetUserWithPassword();
  const string hostPort = GetHostWithPort();
  if (!userPassword.empty() && !hostPort.empty())
    return userPassword + '@' + hostPort;
  else if (!hostPort.empty())
    return hostPort;
  else
    return string();
}

void CProxy::SetHostWithPortUserPassword(
  const string &hostPortUserPassword)
{
  const size_t at_offset = hostPortUserPassword.find('@');
  if (at_offset == string::npos) {
    m_user = m_password = string();
    SetHostWithPort(hostPortUserPassword);
  } else {
    const string user_pass = hostPortUserPassword.substr(0, at_offset);
    const size_t colon_offset = user_pass.find(':');
    if (colon_offset == string::npos) {
      m_user = user_pass;
      m_password = string();
    } else {
      m_user = user_pass.substr(0, colon_offset);
      m_password = user_pass.substr(colon_offset + 1);
    }

    SetHostWithPort(hostPortUserPassword.substr(at_offset + 1));
  }
}

string CProxy::GetHostWithPort() const
{
  std::ostringstream ss;
  if (!m_host.empty()) {
    ss << m_host;
    if (m_port)
      ss << ':' << m_port;
  }
  return ss.str();
}

void CProxy::SetHostWithPort(const string &host)
{
  std::istringstream ss(host);
  std::getline(ss, m_host, ':');
  if (ss.eof())
    m_port = DefaultPorts[m_type];
  else
    ss >> m_port;
}

string CProxy::GetUserWithPassword() const
{
  return m_user.empty() ? string() : (m_user +
    (m_password.empty() ? string() : (string(":") + m_password)));
}

void CProxy::SetUserWithPassword(const string &userpass)
{
  std::istringstream ss(userpass);
  std::getline(ss, m_user, ':');
  std::istreambuf_iterator<char> eos;
  m_password = string(std::istreambuf_iterator<char>(ss), eos);
}

void CProxy::Archive(CArchive& ar)
{
  if (ar.IsStoring())
  {
    ar << (int)m_type;
    ar << m_host;
    ar << m_port;
    ar << m_user;
    ar << m_password;
  }
  else
  {
    int iType;
    ar >> iType;
    m_type = static_cast<CProxy::Type>(iType);
    ar >> m_host;
    ar >> m_port;
    ar >> m_user;
    ar >> m_password;
  }
}
