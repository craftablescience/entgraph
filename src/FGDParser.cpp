#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include "FGDParser.h"

#define SLOME_MAX_STR_CHUNK_LENGTH 1024

using namespace FGD;

char singleTokens[] = "{}[](),:=+";
FGDTokenizer::TokenType valueTokens[] = {FGDTokenizer::OPEN_BRACE, FGDTokenizer::CLOSE_BRACE, FGDTokenizer::OPEN_BRACKET, FGDTokenizer::CLOSE_BRACKET, FGDTokenizer::OPEN_PARENTHESIS, FGDTokenizer::CLOSE_PARENTHESIS, FGDTokenizer::COMMA, FGDTokenizer::COLUMN, FGDTokenizer::EQUALS, FGDTokenizer::PLUS };
enum ParseError tokenErrors[] = { INVALID_OPEN_BRACE, INVALID_CLOSE_BRACE, INVALID_OPEN_BRACKET, INVALID_CLOSE_BRACKET, INVALID_OPEN_PARENTHESIS, INVALID_CLOSE_PARENTHESIS, INVALID_COMMA,INVALID_COLUMN,INVALID_EQUALS,INVALID_PLUS};

bool FGDTokenizer::TokenizeFile(std::string_view file)
{

    if(file.empty())
        return false;

    int pos = 1, ln = 1, i = 0;

    for ( auto iterator = file.cbegin(); iterator != file.cend(); iterator++, i++, pos++ )
    {
        char c = *iterator;

        if ( c == '\t' )
            continue;

        if ( c == '\r' )
            continue;

        if ( c == '\n' )
        {
            ln++;
            pos = 1;
            continue;
        }

        if ( c == '"' )
        {
            int currentLine = ln;
            int currentLength = i;
            int currentPos = pos;
            auto currentIteration = iterator;

            c = '\t'; // We can get away with this to trick the while loop :)
            while ( c != '"' )
            {
                iterator++;
                pos++;
                c = *iterator;
                i++;
                if ( c == '\n' )
                    ln++;
            }

            iterator++;
            i++;
            pos++;
            Token token {};
            token.line = currentLine;
            token.type = STRING;
            token.associatedError = INVALID_STRING;

            int newStrLength = 0;
            token.string = std::string_view{currentIteration, iterator };

            int subtractFromRange = (i - currentLength - token.string .length());

            Range range = { currentPos, pos - (currentPos - subtractFromRange ) };
            token.range = range;


            this->TokenList.push_back(token);
            iterator--;
            i--;
            pos--;
            continue;
        }

        if ( c == '/' && *std::next( iterator ) == '/' )
        {
            int currentLength = i;
            int currentPos = pos;
            auto currentIteration = iterator;

            while ( c != '\n' )
            {
                c = *iterator;
                pos++;
                i++;
                iterator++;
            }
            iterator--;
            i--;
            pos--;

            Token token{};
            token.line = ln;
            token.type = COMMENT;

            token.string = std::string_view {currentIteration, iterator};

            int subtractFromRange = (i - currentLength - token.string .length());

            Range range = { currentPos, pos - (currentPos - subtractFromRange ) };
            token.range = range;

            this->TokenList.push_back(token);

            iterator--;
            i--;
            pos--;
            continue;
        }

        if ( c == '@' )
        {
            int currentLength = i;
            auto currentIteration = iterator;
            int currentPos = pos;

            while ( c != '\n' && c != '\t' && c != '\r' && c != ' ' && c != '(' )
            {
                c = *iterator;
                pos++;
                i++;
                iterator++;
            }
            iterator--;
            i--;
            pos--;

            if ( c == '\n' )
                ln++;
            Token token;
            token.line = ln;
            token.type = DEFINITION;
            token.associatedError = INVALID_DEFINITION;

            int newStrLength = 0;
            token.string = std::string_view{currentIteration, iterator};

            int subtractFromRange = (i - currentLength - newStrLength);

            Range range = { currentPos, pos - (currentPos - subtractFromRange ) };
            token.range = range;

            this->TokenList.push_back(token);

            iterator--;
            i--;
            pos--;
            continue;
        }

        if ( isdigit( c ) != 0 || ( c == '-' && isdigit( *std::next(iterator) ) ) )
        {
            int currentLength = i;
            auto currentIteration = iterator;
            int currentPos = pos;

            if ( c == '-' )
            {
                iterator++;
                pos++;
                i++;
                c = *iterator;
            }

#ifdef SLOME_UNIFIED_FGD
            while ( isdigit( c ) != 0 || c == '.' )
#else
            while ( isdigit( c ) != 0 )
#endif
            {
                c = *iterator;
                i++;
                pos++;
                iterator++;
            }

            iterator--;
            i--;
            pos--;

            Token token;
            token.line = ln;
            token.type = NUMBER;
            token.associatedError = INVALID_NUMBER;
            Range range = { currentPos, pos };
            token.range = range;
            token.string = std::string_view{currentIteration, iterator};

            this->TokenList.push_back(token);

            iterator--;
            i--;
            pos--;
            continue;
        }

        char *valueKey = strchr( singleTokens, c );

        if ( valueKey )
        {
            int spaces = (int)( (int)( (char *)valueKey - (char *)singleTokens ) / sizeof( char ) ); // char should be 1, but I am sanity checking it anyway.
            TokenType tType = valueTokens[spaces];
            enum ParseError tParseError = tokenErrors[spaces];
            Token token;
            token.line = ln;
            token.type = tType;
            token.associatedError = tParseError;
            Range range = { pos, pos + 1 };
            token.range = range;

            token.string = std::string_view { iterator, std::next(iterator) };

            this->TokenList.push_back(token);

            continue;
        }

        if ( c != ' ' )
        {
            int currentLength = i;
            auto currentIteration = iterator;
            int currentPos = pos;

            while ( c != '\n' && c != ' ' && c != '\t' && c != '\r' && !strchr( singleTokens, c ) )
            {
                iterator++;
                pos++;
                c = *iterator;
                i++;
            }

            Token token;
            token.line = ln;
            token.type = LITERAL;
            token.associatedError = INVALID_LITERAL;

            token.string = std::string_view {currentIteration, currentIteration};

            int subtractFromRange = (i - currentLength - token.string.length());

            Range range = { currentPos, pos - (currentPos - subtractFromRange ) };
            token.range = range;

            this->TokenList.push_back(token);

            iterator--;
            i--;
            pos--;
            continue;
        }

    }

    return true;
}

FGD::FGDTokenizer::FGDTokenizer(std::string_view path) {

    std::ifstream file;
    file.open(path.data());

    if (file.is_open())
    {
        file.seekg (0, std::ios::end);
        size_t size = file.tellg();
        std::string fileContents(size, ' ');
        file.seekg (0, std::ios::beg);
        file.read (fileContents.data(), size);
        file.close();

        (std::remove(fileContents.begin(), fileContents.end(), '\r'));

        this->TokenizeFile(fileContents);


    }



}


