#pragma once


#include <string>
#include <vector>

namespace FGD {

    enum ParseError {
        NO_ERROR = 0,
        TOKENIZATION_ERROR,
        INVALID_DEFINITION,
        INVALID_EQUALS,
        INVALID_OPEN_BRACE,
        INVALID_CLOSE_BRACE,
        INVALID_OPEN_BRACKET,
        INVALID_CLOSE_BRACKET,
        INVALID_OPEN_PARENTHESIS,
        INVALID_CLOSE_PARENTHESIS,
        INVALID_COMMA,
        INVALID_STRING,
        INVALID_PLUS,
        INVALID_LITERAL,
        INVALID_COLUMN,
        INVALID_NUMBER,
        ALLOCATION_FAILURE,
        PREMATURE_EOF,
    };

    struct Range {
        int start;
        int end;
    };

    class FGDTokenizer {

    public:
        enum TokenType {

            COMMENT = 0,       // //
            DEFINITION,           // @something
            EQUALS,               // =
            OPEN_BRACE,           // {
            CLOSE_BRACE,       // }
            OPEN_BRACKET,       // [
            CLOSE_BRACKET,       // ]
            OPEN_PARENTHESIS,  // (
            CLOSE_PARENTHESIS, // )
            COMMA,               // ,
            STRING,               // "something"
            PLUS,               // +
            LITERAL,           // anyything that isn't any of the other tokens.
            COLUMN,               // :
            NUMBER,               // numbers -200000 ... 0 ... 2000000

        };

        struct Token {
            TokenType type;
            Range range;
            std::string_view string;
            int line;
            ParseError associatedError;
        };

    public:
        FGDTokenizer(std::string_view path);
    private:
        bool TokenizeFile(std::string_view file);
    private:
        std::vector<Token> TokenList;

    };

    class FGDParser {

        struct ParsingError
        {
            enum ParseError err;
            int line;
            Range span;

        };

#ifdef SLOME_UNIFIED_FGD
        struct TagList
        {
            std::vector<std::string_view> tags;
            int tagCount;
        };
#endif

        struct ClassProperty
        {
            int propertyCount;
            std::vector<std::string_view> properties;

        };

        struct ClassProperties
        {
            std::string_view name;
            int classPropertyCount;
            std::vector<ClassProperty> classProperties;
        };

        enum EntityIOPropertyType
        {
            t_string = 0,
            t_integer,
            t_float,
            t_bool,
            t_void,
            t_script,
            t_vector,
            t_target_destination,
            t_color255,
            t_custom,

        };

        struct Choice
        {
            std::string_view value;
            std::string_view displayName;
#ifdef SLOME_UNIFIED_FGD
            TagList tagList;
#endif
        };

        struct Flag
        {
            int value;
            bool checked;
            std::string_view displayName;
#ifdef SLOME_UNIFIED_FGD
            TagList tagList;
#endif
        } Flag_t;

        struct EntityProperties
        {
            std::string_view propertyName;
            std::string_view type;
            std::string_view displayName;	   // The following 3 are optional and may be empty as a result.
            std::string_view defaultValue;
            std::string_view propertyDescription;
            bool readOnly;
            bool reportable;

#ifdef SLOME_UNIFIED_FGD
            TagList tagList;
#endif

            int choiceCount; // This is a special case if the EntityPropertyType is t_choices
            std::vector<Choice> choices;

            int flagCount; // This is a special case if the EntityPropertyType is t_flags
            std::vector<Flag> flags;

        };

        enum IO
        {
            INPUT = 0,
            OUTPUT,
        };

        struct InputOutput
        {
            std::string_view name;
            std::string_view description;
            IO putType;
            std::string_view stringType;
            EntityIOPropertyType type;
#ifdef SLOME_UNIFIED_FGD
            TagList tagList;
#endif

        };
#ifdef SLOME_UNIFIED_FGD
        struct EntityResource
        {
            std::string_view key;
            std::string_view value;
            TagList tagList;
        };
#endif

        struct Entity
        {
            std::string_view type;
            int classPropertyCount;
            std::vector<ClassProperties> classProperties;
            std::string_view entityName;
            std::string_view entityDescription;
            int entityPropertyCount;
            std::vector<EntityProperties> entityProperties;
            int IOCount;
            std::vector<InputOutput> inputOutput;
#ifdef SLOME_UNIFIED_FGD
            std::vector<EntityResource> resources;
            int resourceCount;
#endif
        };

        struct AutoVisGroupChild
        {
            std::string_view name;
            int childCount;
            std::vector<std::string_view> children;
        };

        struct AutoVIsGroup
        {
            std::string_view name;
            int childCount;
            struct std::vector<AutoVisGroupChild> children;
        };

        struct FGDFile
        {
            Range mapSize;
            int entityCount;
            std::vector<Entity> entities;
            int materialExcludeCount;
            std::vector<std::string_view> materialExclusions;
            int includeCount;
            std::vector<std::string_view> includes;
            int visGroupCount;
            std::vector<AutoVIsGroup> autoVisGroups;
        };
    };

}