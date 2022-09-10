#include <cassert>
#include <dpp/appcommand.h>
#include <dpp/dpp.h>
#include <dpp/guild.h>
#include <dpp/message.h>
#include <dpp/role.h>
#include <dpp/snowflake.h>
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <iostream>
#include "iSerealize.hh"
#include <fstream>

#include <sstream>

typedef std::string AnketaParametr;

typedef std::vector<std::string> ParametrName;

static std::vector<ParametrName> paramNames;

bool hasOperator(char symbol)
{
  const std::string operators = ":.,;!?&()*`\"$#\n";

  for (char op : operators)
  {
    if (op == symbol)
      return true;
  }

  return false;
}

bool hasDelim(std::string delim)
{
  const auto delims = {"-", ":"};

  for (auto &df : delims)
  {
    if (df == delim)
      return true;
  }

  return false;
}

std::vector<std::string> getTokens(std::string content)
{
  std::vector<std::string> result;

  std::size_t i = 0;
  std::size_t prev = 0;
  for (; i < content.size(); i++)
  {
    if (i == content.size() - 1)
    {
      result.push_back(
          content.substr(prev, i - prev + 1));
      break;
    }

    if (content[i] == ' ')
    {
      result.push_back(
          content.substr(prev, i - prev));

      result.push_back(
          {' '});

      prev = i + 1;
    }
    else if (hasOperator(content[i]))
    {
      result.push_back(
          content.substr(prev, i - prev));

      result.push_back(
          {content[i]});

      prev = i + 1;
    }
  }

  return result;
}

const std::string bracketOperator = "[]{}()[]||__**``''\"\"";

bool isBRSpace(std::string::value_type v, bool bg = true)
{
  for (std::string::size_type i = bg ? 0 : 1; i < bracketOperator.size(); i += 2)
  {
    if (v == bracketOperator[i])
      return true;
  }

  return false;
}

bool isNMSpace(std::string::value_type v)
{
  const std::string operators = "0123456789.,;*-=\\/#$@!$%^&'+-=`";

  for (auto e : operators)
  {
    if (v == e)
      return true;
  }

  return false;
}

std::string cutBRSpace(std::string token)
{
  std::string result;

  bool bg = true;
  for (auto& e : token)
  {
    if (bg)
    {
      if (isBRSpace(e, bg))
      {
        bg = false;
      } else
      {
        result.push_back(e);
      }
    } else
    {
      if (isBRSpace(e, bg))
      {
        bg = true;
      }
    }
  }

  return result;
}

std::string cutNMSpace(std::string token)
{
  std::string result;
  for (auto& e : token)
  {
    if (!isNMSpace(e))
    {
      result.push_back(e);
    }
  }

  return cutBRSpace(result);
}

std::string cutBGSpace(std::string token)
{
  int bg = 0;
  int ed = 0;

  for (int i = 0; i < token.size(); i++)
  {
    if (token[i] != ' ')
    {
      bg = i;
      break;
    }
  }

  for (int i = token.size() - 1; i > 0; i--)
  {
    if (token[i] != ' ')
    {
      ed = i;
      break;
    }
  }

  return cutNMSpace(token.substr(bg, ed - bg + 1));
}

bool isFamousPoint(std::string token)
{
  for (auto &e : paramNames)
  {
    for (auto &te : e)
    {
      if (te == token)
        return true;
    }
  }

  return false;
}

struct VaultInfo
{
public:
  std::string name;
  std::string alias;
};

void vaultSerealize(VaultInfo vinfo, std::ostream& os)
{
  strSerealize(vinfo.name, os);
  strSerealize(vinfo.alias, os);
}

void vaultDerealize(VaultInfo& vinfo, std::istream& is)
{
  strDerealize(vinfo.name, is);
  strDerealize(vinfo.alias, is);
}

static std::vector<VaultInfo> vaults;

struct PersInfo : ISerealize
{
public:
  std::unordered_map<std::string, double> moneys;

  void seralize (std::ostream &os) const override
  {
    baseSerealize (moneys.size(), os);
    for (auto& e : moneys)
    {
      strSerealize(e.first, os);
      baseSerealize(e.second, os);
    }
  }

  void derealize (std::istream &is) override
  {
    std::size_t msize = 0;
    baseDerealize(&msize, is);
    for (decltype(msize) i = 0; i < msize; i++)
    {
      std::string key;
      double value;

      strDerealize(key, is);
      baseDerealize(&value, is);

      moneys[key] = value;
    }
  }
};

struct Anketa : ISerealize
{
public:
  std::string content;
  std::vector<std::string> attachments;
  dpp::snowflake message_id;
  dpp::snowflake channel_id;
  dpp::snowflake guild_id;
  std::string message_url;

  std::unordered_map<std::string, AnketaParametr> params;

  PersInfo info;

  void seralize (std::ostream &os) const override
  {
    strSerealize (content, os);

    baseSerealize (attachments.size(), os);
    for (auto a : attachments)
    {
      strSerealize (a, os);
    }

    baseSerealize(message_id, os);
    baseSerealize(channel_id, os);
    baseSerealize(guild_id, os);
    strSerealize(message_url, os);

    baseSerealize(params.size(), os);
    for (auto& e : params)
    {
      strSerealize(e.first, os);
      strSerealize(e.second, os);
    }

    info.seralize (os);
  }

  void derealize (std::istream &is) override
  {
    strDerealize (content, is);
    std::size_t len;
    baseDerealize (&len, is);
    for (decltype(len) i = 0; i < len; i++)
    {
      std::string url;
      strDerealize (url, is);

      attachments.push_back (url);
    }

    baseDerealize(&message_id, is);
    baseDerealize(&channel_id, is);
    baseDerealize(&guild_id, is);
    strDerealize(message_url, is);

    std::size_t psize;
    baseDerealize(&psize, is);
    for (decltype(psize) i = 0; i < psize; i++)
    {
      std::string key, value;
      strDerealize(key, is);
      strDerealize(value, is);

      params[key] = value;
    }
    info.derealize (is);
  }

  Anketa()
  {
  }

  Anketa(std::string content)
  {
    this->content = content;
  }

  std::string& valueof(std::string name)
  {
    for (auto &e : paramNames)
    {
      bool valid = false;
      for (auto &fe : e)
      {
        if (fe == name)
        {
          valid = true;
          break;
        }
      }

      if (valid)
      {
        for (auto &fe : e)
        {
          if (params.contains(fe))
          {
            return params[fe];
          }
        }
      }
    }

    std::string none = "Неизвестно";
    return none;
  }

  static Anketa build(std::string content, std::vector<dpp::attachment> attachments, dpp::snowflake message_id, dpp::snowflake channel_id, dpp::snowflake guild_id)
  {
    Anketa anketa(content);

    auto tokens = getTokens(content);

    std::size_t i = 0;
    bool isNamParam = true;

    std::string namParam;
    std::string valParam;

    std::size_t beginName = 0;

    for (; i < tokens.size(); i++)
    {
      if (isNamParam)
      {
        if (hasDelim(tokens[i]))
        {
          if (isFamousPoint(cutBGSpace(namParam)))
          {
            isNamParam = false;
          }
          else
          {
            isNamParam = true;
            namParam = "";
          }
        }
        else
        {
          if (hasOperator(tokens[i][0]))
          {
            namParam = namParam + tokens[i];
          }
          else
          {
            namParam = namParam + tokens[i];
          }
        }
      }
      else
      {
        if (i == tokens.size() - 1)
        {
          valParam = valParam + tokens[i];
          anketa.params[cutBGSpace(namParam)] = valParam;
          break;
        }

        if (tokens[i] == "\n")
        {
          bool isNextValue = false;

          std::size_t j = i + 1;
          std::string tempName;
          for (; j < tokens.size(); j++)
          {
            if (j == tokens.size() - 1)
            {
              isNextValue = true;
              break;
            }

            if (tokens[j] == "\n")
            {
              isNextValue = true;
              break;
            }
            else if (hasDelim(tokens[j]))
            {
              if (isFamousPoint(cutBGSpace(tempName)))
              {
                isNextValue = false;
                break;
              }
              else
              {
                isNextValue = true;
                break;
              }
            }
            else
            {
              tempName = tempName + tokens[j];
            }
          }

          if (isNextValue)
          {
            continue;
          }
          else
          {
            isNamParam = true;
            anketa.params[cutBGSpace(namParam)] = valParam;
            namParam = "";
            valParam = "";
          }
        }
        else
        {
          if (hasOperator(tokens[i][0]))
          {
            valParam = valParam + tokens[i];
          }
          else
          {
            valParam = valParam + tokens[i];
          }
        }
      }
    }

    for (auto& e : attachments)
    {
      anketa.attachments.push_back (e.url);
    }

    anketa.message_id = message_id;
    anketa.channel_id = channel_id;
    anketa.guild_id = guild_id;
    anketa.message_url = "https://discord.com/channels/" +
                         std::to_string(guild_id) + "/" +
                         std::to_string(channel_id) + "/" +
                         std::to_string(message_id);

    return anketa;
  }
};

struct Member : ISerealize
{
public:
  std::vector<Anketa> anketas;
  std::size_t currentPers = 0;

  Member ()
  {

  }

  void seralize (std::ostream &os) const override
  {
    baseSerealize(anketas.size(), os);
    for (std::size_t i = 0; i < anketas.size(); i++)
    {
      anketas[i].seralize(os);
    }
    baseSerealize(currentPers, os);
  }

  void derealize (std::istream &is) override
  {
    std::size_t asize = 0;
    baseDerealize(&asize, is);
    for (decltype(asize) i = 0; i < asize; i++)
    {
      Anketa anketa;
      anketa.derealize(is);

      anketas.push_back(anketa);
    }
    baseDerealize(&currentPers, is);
  }
};

auto createNameParams() -> void
{
  paramNames = {
      {"Имя"},
      {"Мир", "Фандом", "Мир, фандом"},
      {"Способности", "Способности(с описанием того что они делают)"},
      {"Навыки"},
      {"Выносливость"},
      {"История персонажа", "История", "Личные данные", ""},
      {"Слабости и слабые места", "Слабости", "Слабые места"},
      {"Личные вещи", "Инвентарь"},
      {"Внешний вид", "Аватарка", "Картинка"}};
}

struct AnketaStatus
{
public:
  dpp::snowflake author;
  dpp::snowflake channel_id;
  dpp::snowflake msg_id;
  Anketa anketa;

  dpp::snowflake bot_msg_id;

  short status = 0;
};

static std::unordered_map<dpp::snowflake, Member> members;
static std::unordered_map<dpp::snowflake, AnketaStatus> mstatus;

static dpp::snowflake channel_success;

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or empty string on failure
 */
std::string base64_encode(const unsigned char *src, size_t len)
{
  unsigned char *out, *pos;
  const unsigned char *end, *in;

  size_t olen;

  olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

  if (olen < len)
    return std::string(); /* integer overflow */

  std::string outStr;
  outStr.resize(olen);
  out = (unsigned char *)&outStr[0];

  end = src + len;
  in = src;
  pos = out;
  while (end - in >= 3)
  {
    *pos++ = base64_table[in[0] >> 2];
    *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    *pos++ = base64_table[in[2] & 0x3f];
    in += 3;
  }

  if (end - in)
  {
    *pos++ = base64_table[in[0] >> 2];
    if (end - in == 1)
    {
      *pos++ = base64_table[(in[0] & 0x03) << 4];
      *pos++ = '=';
    }
    else
    {
      *pos++ = base64_table[((in[0] & 0x03) << 4) |
                            (in[1] >> 4)];
      *pos++ = base64_table[(in[1] & 0x0f) << 2];
    }
    *pos++ = '=';
  }

  return outStr;
}

auto isValidVault(std::string vault) -> bool
{
  int symbolFind = 0;
  for (char e : vault)
  {
    if (e <= '9' && e >= '0')
    {
      if (symbolFind != 0)
        return false;
    } else
    {
      symbolFind++;
    }
  }

  return symbolFind <= 3;
}

struct VaultValue
{
 public:
  bool error;
  VaultInfo vinfo;
  double value;
  bool trac = true;
};

auto getVaultValue(std::string& vault) -> VaultValue
{
  std::string val;
  std::string trac;
  int symbolFind = 0;

  std::cout << "EOF>" << vault << std::endl;

  for (int i = 0; i < vault.size(); i++)
  {
    auto e = vault[i];
    std::cout << e << std::endl;
    if (e <= '9' && e >= '0' || e == '.')
    {
      if (symbolFind != 0)
      {
        return VaultValue {.error = true};
      }

      val.push_back (e);
    } else
    {
      symbolFind++;
      trac.push_back (e);
    }
  }

  std::cout << "{val = " << val << ", trac = " << trac << "}" << std::endl;

  if (trac.size() > 3)
  {
    return VaultValue {.error = true};
  }

  if (trac.size() == 0)
  {
    double tval = std::stod (val);

    return VaultValue{.error = false, .vinfo = {}, .value = tval, .trac = false};
  } else
  {
    VaultInfo vinfo;
    for (auto v : vaults)
    {
      if (v.alias == trac)
        vinfo = v;
    }

    if (vinfo.alias.size () == 0)
    {
      return VaultValue{.error = true};
    }

    double tval = std::stod (val);

    return VaultValue{.error = false, .vinfo = vinfo, .value = tval};
  }
}

static std::string mainVault;

std::unordered_map<dpp::snowflake, std::string> whs;

auto saveAll(std::ostream& os) -> void
{
  // TODO: here

  strSerealize(mainVault, os);
  baseSerealize(channel_success, os);

  baseSerealize (vaults.size(), os);
  for (auto& vinfo : vaults)
  {
    vaultSerealize(vinfo, os);
  }

  baseSerealize (members.size(), os);
  for (auto& member : members)
  {
    baseSerealize (member.first, os);
    member.second.seralize (os);
  }

  // baseSerealize(whs.size(), os);
  // for (auto& member : whs)
  // {
  //   baseSerealize (member.first, os);
  //   strSerealize (member)
  // }
}

void loadAll(std::istream& is)
{
  strDerealize(mainVault, is);
  baseDerealize(&channel_success, is);

  std::size_t vsize = 0;
  baseDerealize (&vsize, is);
  for (decltype(vsize) i = 0; i < vsize; i++)
  {
    VaultInfo vinfo;
    vaultDerealize (vinfo, is);
    vaults.push_back (vinfo);
  }

  std::size_t msize = 0;
  baseDerealize (&msize, is);
  for (decltype(msize) i = 0; i < msize; i++)
  {
    dpp::snowflake mid;
    baseDerealize (&mid, is);
    Member member;
    member.derealize (is);

    members[mid] = member;
  }

  // baseDerealize (&msize, is);
  // for (decltype(msize) i = 0; i < msize; i++)
  // {
  //   dpp::snowflake mid;
  //   baseDerealize (&mid, is);

  //   std::string 

  //   whs[mid] = member;
  // }
}

// DEPLOY: 
const dpp::snowflake application_id = 1009119133046149121;

// TEST:
//const dpp::snowflake application_id = 1018047745535254628;

auto main(int argc, char const *argv[]) -> int
{
  const std::string token(argv[1]);

  createNameParams();

  std::ifstream fis = std::ifstream("rolebot.dat");
  if (fis.is_open())
  {
    loadAll(fis);
  }

  dpp::cluster bot(token);

  bot.on_log(dpp::utility::cout_logger());

  bot.on_button_click([&bot](const dpp::button_click_t& event) -> void
  {
    const auto bid = event.custom_id;
    std::string type;
    dpp::snowflake mid;

    bool findRole = false;
    for (const auto r : event.command.member.roles)
    {
      
      if (r == 1017431162479706132 || 
          r == 1008078725985865830 ||
          r == 1008080963856760914)
      {
        findRole = true;
        break;
      }
    }

    if (!findRole)
    {
      return;
    }

    for (std::size_t i = 0; i < bid.size(); i++)
    {
      if (bid[i] == ';')
      {
        mid = std::stoul(bid.substr(0, i));
        type = bid.substr(i + 1, bid.size() - i);
        break;
      }
    }

    AnketaStatus status = mstatus[mid];
    Member& member = members[status.author];
    Anketa& anketa = status.anketa;

    for (auto vinfo : vaults)
    {
      anketa.info.moneys[vinfo.name] = 0.0;
    }

    if (type == "ins_succ")
    {
      member.anketas.push_back (anketa);

      bot.message_create(dpp::message (
        channel_success, 
        "<@" + std::to_string(status.author) + "> -> " + anketa.valueof("Имя")
      ));

      bot.message_delete(status.bot_msg_id, status.channel_id);
    } else
    {
      bot.message_create(dpp::message(
        status.channel_id,
        "<@" + std::to_string(status.author) + ">, ваша анкету не приняли. Причину укажет анкетолог."
      ).set_reference(status.msg_id));

      bot.message_delete(status.bot_msg_id, status.channel_id);
    }
  });

  bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) -> void
                      {
      std::cout << event.command.get_command_name() << std::endl;

      if (event.command.get_command_name() == "inspect")
      {
          const dpp::snowflake channel_id = event.command.channel_id;
          //const dpp::snowflake message_id = std::stol(std::get<std::string>(event.get_parameter("msgid")));

          //bot.message_get_sync()

          std::string content {};
          dpp::message msg;
          std::vector<dpp::attachment> attchs;

          for (std::size_t i = 1; i < 6; i++)
          {
            std::string pid;
            if (i == 1)
              pid = "";
            else
              pid = std::to_string(i);

            auto param = event.get_parameter("msgid" + pid);
            auto msgid = std::get_if<std::string>(&param);
            if (msgid == nullptr)
              continue;

            msg = bot.message_get_sync(std::stol(*msgid),  channel_id);
            content = content + msg.content;

            for (auto& ae : msg.attachments)
              attchs.push_back(ae);
          }

          Anketa anketa = Anketa::build(content, attchs, msg.id, event.command.channel_id, event.command.guild_id);

          if (!members.contains (msg.author.id))
          {
            Member member;

            members[msg.author.id] = member;
          }
          // } else
          // {
          //   members[msg.author.id].anketas.push_back (anketa);
          // }

          dpp::embed emm;
          emm.set_title ("Анкета");

          std::string cnt;
          for (const std::pair<const std::string, std::string>& n : anketa.params)
          {
            cnt = n.first + "\n---\n" + n.second + "\n\n" + cnt;
          }

          emm.set_description (cnt);

          for (auto& attach : anketa.attachments)
          {
            emm.set_image (attach);
          }

          std::string mid = std::to_string (msg.id);

          AnketaStatus& st = mstatus[msg.id] = AnketaStatus {msg.author.id, msg.channel_id, msg.id};
          st.anketa = anketa;

          bot.message_create (
            dpp::message (event.command.channel_id, emm)
            .add_component(
              dpp::component().add_component(
                  dpp::component().set_label("Принято!").
                  set_type(dpp::cot_button).
                  set_emoji("🟢").
                  set_style(dpp::cos_success).
                  set_id(mid + ";ins_succ")
              )
            )
            .add_component(
              dpp::component().add_component(
                  dpp::component().set_label("Отказано.").
                  set_type(dpp::cot_button).
                  set_emoji("🔴").
                  set_style(dpp::cos_danger).
                  set_id(mid + ";ins_fail")
              )
            ),
            [&st](const dpp::confirmation_callback_t& cb2) -> void
            {
              st.bot_msg_id = std::get<dpp::message>(cb2.value).id;
            }
          );

          event.reply("Делаю дело...");
      } else
      if (event.command.get_command_name() == "visitka")
      {
        auto mention = std::get<dpp::snowflake>(event.get_parameter ("author"));

        if (!members.contains (mention))
        {
          members[mention] = Member{};
          event.reply ("У этого парня нет анкет.");
          return;
        }
        
        auto member = members[mention];
        auto arg = std::get<int64_t>(event.get_parameter ("id"));

        if (member.anketas.size() <= arg)
        {
          event.reply ("У этого парня нет такой анкеты!");
          return;
        }

        Anketa anketa = member.anketas[arg];

        dpp::embed emm;
        emm.set_title (anketa.valueof ("Имя"));
        emm.set_description (
            "**Способности**:\n" +
            anketa.valueof ("Способности") + "\n\n" +
            "**Слабости**:\n" +
            anketa.valueof ("Слабости") + "\n\n" +
            "Ссылка на анкету: " +
            anketa.message_url
        );

        for (auto& e : anketa.attachments)
        {
          emm.set_image (e);
        }

        bot.message_create (
            dpp::message (event.command.channel_id, emm)
        );

        event.reply("Вот вам его визиточка.");
      } else
      if (event.command.get_command_name() == "dp")
      {
        if (!members.contains (event.command.usr.id))
        {
          members[event.command.usr.id] = Member{};
          event.reply ("У этого парня нет анкет.");
          return;
        }
        auto member = members[event.command.usr.id];
        Anketa anketa = member.anketas[member.currentPers];

        if (!whs.contains(event.command.channel_id))
        {
          event.reply("В этом канале нельзя оставлять сообщения!");
          return;
        }

        dpp::webhook wh (whs[event.command.channel_id]);
        wh.name = anketa.valueof("Имя");

        if (anketa.attachments.size() != 0)
        {
          wh.avatar = anketa.attachments[0];
        }

        bot.execute_webhook (wh, dpp::message(std::get<std::string>(event.get_parameter ("msg"))), true);
      } else
      if (event.command.get_command_name() == "set_pers")
      {
        if (!members.contains (event.command.usr.id))
        {
          members[event.command.usr.id] = Member{};
          event.reply ("У этого парня нет анкет.");
          return;
        }

        auto id = std::get<int64_t>(event.get_parameter ("id"));;

        Member& member = members[event.command.usr.id];
        if (member.anketas.size() <= id)
        {
          event.reply ("У вас анкет не больше, чем вы указали.");
          return;
        }

        member.currentPers = id;
      } else
      if (event.command.get_command_name() == "anketas")
      {
        auto val = event.get_parameter ("author");
        dpp::snowflake* id = std::get_if<dpp::snowflake>(&val);

        if (id == nullptr)
        {
          dpp::snowflake try_id = event.command.usr.id;
          id = &try_id;
        }

        if (!members.contains (*id))
        {
          members[*id] = Member{};
          event.reply ("У этого парня нет анкет.");
          return;
        }

        Member& member = members[*id];

        dpp::embed emm;
        emm.set_title ("Анкеты");
        std::string str;
        int i = 0;

        for (auto kk : member.anketas)
        {
          str = str + std::to_string(i) + ". " + kk.valueof ("Имя") + "\n";
          i++;
        }

        emm.set_description (str);
        bot.message_create (
          dpp::message (event.command.channel_id, emm)
        );
      } else
      if (event.command.get_command_name() == "set_success")
      {
        for (auto r : event.command.member.roles)
        {
          if (r == 1017431162479706132 || 
              r == 1008078725985865830 ||
              r == 1008080963856760914)
          {

            channel_success = std::get<dpp::snowflake>(event.get_parameter("channel"));
            event.reply ("Канал установлен.");
          }
        }
      } else
      if (event.command.get_command_name() == "whbind")
      {
        for (auto r : event.command.member.roles)
        {
          if (r == 1017431162479706132 || 
              r == 1008078725985865830 ||
              r == 1008080963856760914)
          {

            whs[std::get<dpp::snowflake>(event.get_parameter("channel"))] = std::get<std::string>(event.get_parameter("url"));
            event.reply ("Канал-вебхук установлен.");
          }
        }
      } else
      if (event.command.get_command_name() == "rename")
      {
        if (!members.contains(event.command.usr.id))
        {
            event.reply (dpp::ir_channel_message_with_source, "У вас нет анкет.");
            return;
        }

        Member& member = members[event.command.usr.id];
        if (member.anketas.size() == 0)
        {
            event.reply (dpp::ir_channel_message_with_source, "У вас нет анкет.");
            return;
        }

        int64_t id = std::get<int64_t>(event.get_parameter("id"));
        std::string name = std::get<std::string>(event.get_parameter("name"));

        if (member.anketas.size() <= id)
        {
          event.reply(dpp::ir_channel_message_with_source, "У вас не больше анкет, чем вы указали.");
        }

        Anketa& anketa = member.anketas[id];
        std::string& namef = anketa.valueof("Имя");
        namef = name;

        event.reply("Имя анкеты изменено на `" + name + "`!");
      } else
      if (event.command.get_command_name() == "cvault")
      {
        bool findRole = false;

        for (auto r : event.command.member.roles)
        {
          if (r == 1017431162479706132 || 
              r == 1008078725985865830 ||
              r == 1008080963856760914)
          {
            findRole = true;
            break;
          }
        }

        if (!findRole)
          return;

        std::string v = std::get<std::string>(event.get_parameter("name"));
        std::string a = std::get<std::string>(event.get_parameter("alias"));

        if (a.size() > 3)
        {
          event.reply ("Обозначение не может быть длиннее трёх символов!");
          return;
        }

        for (auto& e : vaults)
        {
          if (e.name == v)
          {
            event.reply ("Такая валюта уже существует...");
            return;
          }

          if (e.alias == a)
          {
            event.reply ("Такое обозначение валюты уже существует.");
          }
        }

        VaultInfo vinfo = VaultInfo { .name = v, .alias = a };
        vaults.push_back (vinfo);

        for (auto& member : members)
        {
          for (auto& anketa : member.second.anketas)
            anketa.info.moneys[vinfo.name] = 0;
        }

        dpp::embed em;
        em.title = "Операция завершена.";
        em.description = "Валюта \"**" + v + "**\" была создана.";

        mainVault = v;

        event.reply (dpp::message(event.command.channel_id, em));
      } else
      if (event.command.get_command_name() == "gave")
      {
        bool findRole = false;

        for (auto r : event.command.member.roles)
        {
          if (r == 1017431162479706132 || 
              r == 1008078725985865830 ||
              r == 1008080963856760914)
          {
            findRole = true;
            break;
          }
        }

        if (!findRole)
          return;

        // from, id, value
        dpp::snowflake from = std::get<dpp::snowflake>(event.get_parameter ("from"));
        int64_t id = std::get<int64_t>(event.get_parameter("id"));
        std::string strval = std::get<std::string>(event.get_parameter("value"));

        std::cout << "\"" << strval << "\"" << std::endl;

        auto vinfo = getVaultValue(strval);
        if (vinfo.error)
        {
          event.reply ("Прочитать значение не было возможным...");
          return;
        }

        if (!vinfo.trac)
        {
          bool hasFind = false;
          VaultInfo tvinfo;
          for (auto e : vaults)
          {
            if (e.name == mainVault)
            {
              tvinfo = e;
              hasFind = true;
              break;
            }
          }

          if (!hasFind)
          {
            event.reply ("Вы ввели значение без валюты, а главная валюта не назначена!");
            return;
          }

          vinfo.vinfo = tvinfo;
          vinfo.trac = false;
        }

        if (!members.contains(from))
        {
          event.reply ("Автор не регистрирован на сервере!");
          return;
        }

        Member& member = members[from];
        if (member.anketas.size() <= id)
        {
          event.reply ("Такой анкеты нет.");
          return;
        }

        member.anketas[id].info.moneys[vinfo.vinfo.name] += vinfo.value;

        dpp::embed em;
        em.title = "Валюта";
        em.description = "Персонажу **" + member.anketas[id].valueof("Имя") + "** было начислено " + strval;

        event.reply (dpp::message(event.command.channel_id, em));
      } else
      if (event.command.get_command_name() == "bal")
      {
        dpp::snowflake from = std::get<dpp::snowflake>(event.get_parameter("from"));
        int64_t id = std::get<int64_t>(event.get_parameter("id"));

        if (!members.contains (from))
        {
          event.reply ("Этот пользователь ещё не пользовался функциями бота.");
          return;
        }

        Member& member = members[from];
        if (member.anketas.size() <= id)
        {
          event.reply("Нет такой анкеты.");
          return;
        }

        dpp::embed em;

        std::string res;
        for (auto& e : member.anketas[id].info.moneys)
        {
          res = res + e.first;
          res = res + " = " + std::to_string (e.second) + "\n";
        }

        em.title = "Баланс";
        em.description = res;

        event.reply (dpp::message(event.command.channel_id, em));
      } else
      if (event.command.get_command_name() == "swap")
      {
        if (!members.contains (event.command.usr.id))
        {
          event.reply ("У вас нет персонажей.");
          return;
        }

        int64_t from_id = std::get<int64_t> (event.get_parameter ("from_id"));
        dpp::snowflake to_id = std::get<dpp::snowflake> (event.get_parameter ("to"));
        int64_t id = std::get<int64_t> (event.get_parameter ("id"));
        std::string strval = std::get<std::string>(event.get_parameter("value"));

        if (!members.contains(to_id))
        {
          event.reply("У получателя нет персонажей.");
          return;
        }

        Member& from = members[event.command.usr.id];
        Member& to = members[to_id];

        if (from.anketas.size() <= from_id)
        {
          event.reply("Такого персонажа у вас нет!");
          return;
        }

        if (to.anketas.size() <= id)
        {
          event.reply("Такого персонажа нет у @<" + std::to_string(to_id) + ">!");
          return;
        }

        auto vinfo = getVaultValue(strval);
        if (vinfo.error)
        {
          event.reply ("Прочитать значение валюты не было возможным...");
          return;
        }

        if (!vinfo.trac)
        {
          bool hasFind = false;
          VaultInfo tvinfo;
          for (auto e : vaults)
          {
            if (e.name == mainVault)
            {
              tvinfo = e;
              hasFind = true;
              break;
            }
          }

          if (!hasFind)
          {
            event.reply ("Вы ввели значение без валюты, а главная валюта не назначена!");
            return;
          }

          vinfo.vinfo = tvinfo;
          vinfo.trac = false;
        }

        if (from.anketas[from_id].info.moneys[vinfo.vinfo.name] < vinfo.value)
        {
          event.reply("У персонажа недостаточно средств!");
          return;
        }

        from.anketas[from_id].info.moneys[vinfo.vinfo.name] -= vinfo.value;
        to.anketas[id].info.moneys[vinfo.vinfo.name] += vinfo.value;

        dpp::embed em;
        em.title = "Валюта";
        em.description =
            "Валюта была передена! Балансы:\n**" +
            from.anketas[from_id].valueof("Имя") + "**: " +
            std::to_string(from.anketas[from_id].info.moneys[vinfo.vinfo.name]) + "\n**" +
            to.anketas[id].valueof("Имя") + "**: " +
            std::to_string(to.anketas[from_id].info.moneys[vinfo.vinfo.name]);

        event.reply (dpp::message (event.command.channel_id, em));
      } else
      if (event.command.get_command_name() == "sav")
      {
        std::cout << "Сохраняю данные..." << std::endl;
        std::ofstream ff("rolebot.dat");
        saveAll(ff);
        std::cout << "Загружаю данные..." << std::endl;

        event.reply("Все данные сохранены!");
      }
    });

  bot.on_ready([&bot](const dpp::ready_t &event) -> void
               {
      dpp::slashcommand inspect ("inspect", "Читает тест, переводя в анкету.", application_id);
      inspect.add_option (
          dpp::command_option (dpp::co_string, "msgid", "Идентификатор сообщения анкеты", true)
      );
      inspect.add_option (
          dpp::command_option (dpp::co_string, "msgid2", "Идентификатор сообщения анкеты", false)
      );
      inspect.add_option (
          dpp::command_option (dpp::co_string, "msgid3", "Идентификатор сообщения анкеты", false)
      );
      inspect.add_option (
          dpp::command_option (dpp::co_string, "msgid4", "Идентификатор сообщения анкеты", false)
      );
      inspect.add_option (
          dpp::command_option (dpp::co_string, "msgid5", "Идентификатор сообщения анкеты", false)
      );
      bot.global_command_create(inspect);

      dpp::slashcommand visitka ("visitka", "Даёт визитку анкеты.", application_id);
      visitka.add_option (
        dpp::command_option (dpp::co_mentionable, "author", "Идентификатор автора анкеты", true)
      );
      visitka.add_option (
          dpp::command_option (dpp::co_integer, "id", "Номер анкеты")
      );
      bot.global_command_create(visitka);

      dpp::slashcommand dp ("dp", "Написать от имени персонажа", application_id);
      dp.add_option (
          dpp::command_option (dpp::co_string, "msg", "Текст", true)
      );
      bot.global_command_create(dp);

      dpp::slashcommand set_pers ("set_pers", "Выбрать текущего активного персонажа", application_id);
      set_pers.add_option (
          dpp::command_option (dpp::co_integer, "id", "Номер анкеты", true)
      );
      bot.global_command_create(set_pers);

      dpp::slashcommand anketas ("anketas", "Выводит список анкет", application_id);
      anketas.add_option (
          dpp::command_option (dpp::co_mentionable, "author", "Автор анкеты (указывать только тогда, когда нужно узнать список у других)", false)
      );
      bot.global_command_create(anketas); 

      dpp::slashcommand set_success ("set_success", "Устанавливает канал, куда будут высылаться принятые анкеты.", application_id);
      set_success.add_option (
        dpp::command_option (dpp::co_channel, "channel", "Канал, куда будут высылаться принятые анкеты.", true)
      );
      bot.global_command_create(set_success);

      dpp::slashcommand whbind ("whbind", "Устанавливает вебхук под канал", application_id);
      whbind.add_option(
        dpp::command_option (dpp::co_channel, "channel", "Канал для вебхука", true)
      );
      whbind.add_option(
        dpp::command_option(dpp::co_string, "url", "Ссылка вебхука", true)
      );
      bot.global_command_create(whbind);

      dpp::slashcommand rename ("rename", "Переменовывает персонажа", application_id);
      rename.add_option (
        dpp::command_option(dpp::co_integer, "id", "Номер анкеты в списке", true)
      );
      rename.add_option(
        dpp::command_option(dpp::co_string, "name", "Новое имя для персонажа", true)
      );
      bot.global_command_create(rename);

      dpp::slashcommand cvault ("cvault", "Создаёт выделенную валюту", application_id);
      cvault.add_option(
        dpp::command_option(dpp::co_string, "name", "Имя валюту", true)
      );
      cvault.add_option(
        dpp::command_option(dpp::co_string, "alias", "Префикс валюты для указываний цен.", true)
      );
      bot.global_command_create(cvault);

      dpp::slashcommand gave ("gave", "Безвозмезднео выдать выделенную валюту персонажу", application_id);
      gave.add_option(
        dpp::command_option(dpp::co_mentionable, "from", "Кому выдать валюту", true)
      );
      gave.add_option(
        dpp::command_option(dpp::co_integer, "id", "Номер персонажа в списке", true)
      );
      gave.add_option(
        dpp::command_option(dpp::co_string, "value", "Сколько выдать (14045ru к примеру)", true)
      );
      bot.global_command_create(gave);

      dpp::slashcommand bal ("bal", "Выдаёт баланс персонажа", application_id);
      bal.add_option(
          dpp::command_option(dpp::co_mentionable, "from", "Владелец персонажа", true)
      );
      bal.add_option(
          dpp::command_option(dpp::co_integer, "id", "Персонаж", true)
      );
      bot.global_command_create(bal);

      dpp::slashcommand swapcmd ("swap", "Перевод денег между персонажами", application_id);
      swapcmd.add_option(
        dpp::command_option(dpp::co_integer, "from_id", "Ваш персонаж в списке", true)
      );
      swapcmd.add_option(
        dpp::command_option(dpp::co_mentionable, "to", "Владелец персонажа, которому нужно перевести", true)
      );
      swapcmd.add_option(
        dpp::command_option(dpp::co_integer, "id", "Персонаж, которому нужно перевести", true)
      );
      swapcmd.add_option(
        dpp::command_option(dpp::co_string, "value", "Сколько нужно перевести", true)
      );
      bot.global_command_create(swapcmd);

      dpp::slashcommand sav = dpp::slashcommand("sav", "save all data", application_id);
      bot.global_command_create(sav);
    });

  bot.start(dpp::st_wait);

  return 0;
}
