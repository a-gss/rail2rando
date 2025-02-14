#pragma once

bool regex_match(const char *str, regex_t *regex, int n_capturing_group, char result[][64])
{
    regmatch_t matches[n_capturing_group + 1];  // Index 0 = full match

    if (regexec(regex, str, n_capturing_group + 1, matches, 0) == 0) {
        if (n_capturing_group == 0) {
            int start = matches[0].rm_so;
            int end = matches[0].rm_eo;
            int len = end - start;

            strncpy(result[0], str + start, len);
            result[0][len] = '\0';
            printf(GREEN "MATCH: %s\n" RESET, result[0]);

        } else {
            printf(GREEN "MATCH: ");
            for (int i = 1; i <= n_capturing_group; i++) {
                if (matches[i].rm_so != -1) {
                    int start = matches[i].rm_so;
                    int end = matches[i].rm_eo;
                    int len = end - start;

                    strncpy(result[i - 1], str + start, len);
                    result[i - 1][len] = '\0';

                    (n_capturing_group > 1) ? printf("(%d)%s ", i, result[i - 1])
                                            : printf("%s ", result[i - 1]);
                }
            }
            puts(RESET);
        }
        info("Regex found in '%s'\n", str);
        return true;
    }

    return false;
}


bool regex_find(const char *pattern, char *mmaped_data, int n_capturing_group, char result[][64])
{
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
