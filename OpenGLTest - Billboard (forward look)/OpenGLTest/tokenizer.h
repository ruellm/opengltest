#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string>
#include <vector>

typedef std::vector<std::string> Tokens;

void Tokenize(std::string& str,Tokens& tokens,
		const std::string& delimiters);
#endif