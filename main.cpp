#include <stdexcept>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef WIN_NT
#include <io.h>
#endif

#include "ibase.h"

class Message
{
private:
	unsigned code;
	unsigned number;
	const char *symbol;
	const char *sqlClass;
	const char *sqlSubClass;
	const char *text;
	bool complete;

public:
	Message(unsigned code, unsigned number, const char *symbol, const char *sqlClass,
			const char *sqlSubClass, const char *text, bool complete) : code(code), number(number), symbol(symbol), sqlClass(sqlClass),
																		sqlSubClass(sqlSubClass), text(text), complete(complete)
	{
	}

public:
	unsigned getCode() { return code; }
	unsigned getNumber() { return number; }
	const char *getSymbol() { return symbol; }
	const char *getSqlClass() { return sqlClass; }
	const char *getSqlSubClass() { return sqlSubClass; }
	const char *getText() { return text; }
	bool isComplete() { return complete; }
};

static std::string SPECIAL_SAVE_CHARS = "=: \t\r\n\f#!";
static char hex_digit[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static char toHex(int nibble)
{
	return hex_digit[nibble & 0xF];
}

static void generate_messages(std::vector<Message> messages, const std::string &msg_filename, const std::string &states_filename)
{
	FILE *msg_file = fopen(msg_filename.c_str(), "w+");
	FILE *states_file = fopen(states_filename.c_str(), "w+");

	for (auto &message : messages)
	{
		unsigned number = (unsigned)FB_IMPL_MSG_ENCODE(message.getNumber(), message.getCode());

		std::string text(message.getText());

		size_t pos = text.find("@", 0);
		while (pos != std::string::npos)
		{
			try
			{
				const auto idx = std::atoi(text.substr(pos + 1, 1).c_str()) - 1;
				if (idx != -1)
				{
					text.replace(pos, 1, "{");
					text.replace(pos + 1, 1, std::to_string(idx));
					text.insert(pos + 2, "}");
				}
				pos = text.find("@", pos + 2);
			}
			catch (const std::exception &ignored)
			{
			}
		}

		std::size_t len = text.length();
		std::string out;

		for (int x = 0; x < len; x++)
		{
			char sym = text[x];

			switch (sym)
			{
			case ' ':
				if (x == 0)
					out.append("\\");

				out.append(" ");
				break;
			case '\\':
				out.append("\\");
				out.append("\\");
				break;
			case '\t':
				out.append("\\");
				out.append("t");
				break;
			case '\n':
				out.append("\\");
				out.append("n");
				break;
			case '\r':
				out.append("\\");
				out.append("r");
				break;
			case '\f':
				out.append("\\");
				out.append("f");
				break;
			default:
				if ((sym < 0x0020) || (sym > 0x007e))
				{
					out.append("\\");
					out.append("u");
					out.append(1, toHex(sym >> 12));
					out.append(1, toHex(sym >> 8));
					out.append(1, toHex(sym >> 4));
					out.append(1, toHex(sym));
				}
				else
				{
					size_t pos = SPECIAL_SAVE_CHARS.find(sym, 0);
					if (pos != std::string::npos)
					{
						out.append("\\");
					}
					out.append(1, sym);
				}
			}
		}

		fprintf(msg_file, "%u=%s\n",
				number,
				out.c_str());

		std::string state(message.getSqlClass());
		state.append(message.getSqlSubClass());

		if (!state.empty())
		{
			fprintf(states_file, "%u=%s\n",
					number,
					state.c_str());
		}
	}

	fclose(msg_file);
	fclose(states_file);
}

int main(int argc, char **argv)
{
	std::vector<Message> messages;

	std::string msg_filename;
	std::string states_filename;

	if (argc < 3)
	{
		msg_filename = "isc_error_msg.properties";
		states_filename = "isc_error_sqlstates.properties";
	}
	else
	{
		msg_filename = argv[1];
		states_filename = argv[2];
	}

#define FB_IMPL_MSG_NO_SYMBOL(facility, number, text) \
	Message{FB_IMPL_MSG_FACILITY_##facility, number, "", "", "", text, false},

#define FB_IMPL_MSG_SYMBOL(facility, number, symbol, text) \
	Message{FB_IMPL_MSG_FACILITY_##facility, number, #symbol, "", "", text, false},

#define FB_IMPL_MSG(facility, number, symbol, sqlCode, sqlClass, sqlSubClass, text) \
	Message{FB_IMPL_MSG_FACILITY_##facility, number, #symbol, sqlClass, sqlSubClass, text, true},

	messages.insert(messages.end(), {
#include "firebird/impl/msg/all.h"
									});

#undef FB_IMPL_MSG_NO_SYMBOL
#undef FB_IMPL_MSG_SYMBOL
#undef FB_IMPL_MSG

	generate_messages(messages, msg_filename, states_filename);

	return 0;
}