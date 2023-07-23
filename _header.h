//allowed max path length for executables
#define MAX_NAME_LEN 1000
WCHAR* removeExecPath(WCHAR* s){
	if (*s == '"') {
		++s;
		while (*s)
			if (*s++ == '"') break;
	}
	else {
		while (*s && *s != ' ' && *s != '\t') ++s;
	}
	while (*s == ' ' || *s == '\t') s++;
	return s;
}