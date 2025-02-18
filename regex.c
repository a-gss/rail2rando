#pragma once

bool regex_match(const char *str, regex_t *regex, int n_capturing_group, char **match)
{
    regmatch_t regmatches[n_capturing_group + 1];  // Index 0 = full match

    if (regexec(regex, str, n_capturing_group + 1, regmatches, 0) != 0) {
        return false; // No match
    }

    if (n_capturing_group <= 1) {
        // Allocate new memory for the full match or single capturing group
        int start = regmatches[n_capturing_group].rm_so;
        int end = regmatches[n_capturing_group].rm_eo;
        int len = end - start;

        *match = malloc(len + 1);
        if (*match == NULL) {
            error("Memory allocation failed!");
            exit(EXIT_FAILURE);
        }

        strncpy(*match, str + start, len);
        (*match)[len] = '\0'; // Null-terminate

    } else {
        // Allocate space for capturing groups
        for (int i = 1; i <= n_capturing_group; i++) {
            if (regmatches[i].rm_so != -1) {
                int start = regmatches[i].rm_so;
                int end = regmatches[i].rm_eo;
                int len = end - start;

                match[i - 1] = malloc(len + 1);
                if (match[i - 1] == NULL) {
                    error("Memory allocation failed!");
                    exit(EXIT_FAILURE);
                }

                strncpy(match[i - 1], str + start, len);
                match[i - 1][len] = '\0';
            }
        }
    }


/*
for (int i = 1; i <= n_capturing_group; i++) {
    if (regmatches[i].rm_so != -1) {
        int start = regmatches[i].rm_so;
        int end = regmatches[i].rm_eo;
        int len = end - start;

        strncpy(match[i - 1], str + start, len);
        match[i - 1][len] = '\0';

        //(n_capturing_group > 1) ? printf("(%d)%s ", i, match[i - 1]) : printf("%s ", match[i - 1]);
    }
}
*/


    //info("Regex found in '%s'\n", str);
    return true; // Match
}

// Store the regex result in result[][]
int regex_find(const char *pattern, char *mmaped_data, int n_capturing_group, char ***result)
{
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        putchar('\n');
        error("Erreur de compilation du regex '%s'", pattern);
        exit(EXIT_FAILURE);
    }

    // result => array du nombre d'occurence du pattern dans le fichier

    int match_count = 0;
    char *eof = memchr(mmaped_data, '\0', SIZE_MAX); // Find the end of the file
    char *start = mmaped_data; // Start at the beginning

    *result = NULL; // Ensure it's NULL initially

    // Extract the line
    while (start < eof) {
        char *end = memchr(start, '\n', eof - start);
        size_t len = end - start;
        char *line = malloc(len + 1); // + \0
        if (!line) {
            error("Memory allocation failed!");
            exit(EXIT_FAILURE);
        }

        line = memcpy(line, start, len);
        line[len-1] = '\0'; // remove the \n from the string
        start = end + 1;    // move to the next line

        char *match = NULL;
        if(regex_match(line, &regex, n_capturing_group, &match)) {
            // reallocate for 1 more string to be stored
            *result = realloc(*result, (match_count + 1) * sizeof(char *));
            if (!*result) {
                error("Memory allocation failed!");
                exit(EXIT_FAILURE);
            }

            (*result)[match_count] = match;  // Store new match
            match_count++;
        }

        free(line);
    }

    regfree(&regex);
    match_count > 0 ? printf(GREEN "%d MATCHES\n" RESET, match_count) : puts(RED "NO MATCH" RESET);

    return match_count;
}

char *escape_regex(const char *input) {
    // Special characters that need to be escaped in regex
    const char *special_chars = ":|";
    size_t length = strlen(input);
    size_t new_len = length;

    // Count how many characters we need to escape
    for (size_t i = 0; i < length; i++) {
        if (strchr(special_chars, input[i]) != NULL) {
            new_len++;
        }
    }

    // Allocate memory for the new escaped string
    char *escaped = (char *)malloc(new_len + 1); // +1 for null terminator
    if (escaped == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    size_t j = 0;
    for (size_t i = 0; i < length; i++) {
        if (strchr(special_chars, input[i]) != NULL) {
            escaped[j++] = '\\';  // Add escape character before special char
        }
        escaped[j++] = input[i];
    }
    escaped[j] = '\0';  // Null terminate the new string
    return escaped;
}

/*
bool regex_find_once(const char *pattern, char *mmaped_data, int n_capturing_group, char **result)
{
    // Stops after the first match

    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        error("Erreur de compilation du regex '%s'", pattern);
        return false;
    }

    char *eof = memchr(mmaped_data, '\0', SIZE_MAX); // Find the end of the file
    char *start = mmaped_data; // Start at the beginning

    while (start < eof) {
        // Extract the line
        char * end = memchr(start, '\n', eof - start);
        size_t len = end - start;
        char *line = malloc(len + 1); // + \0
        line = memcpy(line, start, len);
        line[len-1] = '\0'; // remove the \n from the string
        start = end + 1;    // move to the next line

        if (regex_match(line, &regex, n_capturing_group, result)) {
            free(line);
            regfree(&regex);
            return true;
        }

        free(line);
    }

    regfree(&regex);
    puts(RED "NO MATCH" RESET);
    return false;
}
*/
