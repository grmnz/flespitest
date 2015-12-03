#include <stdio.h>

/*
Configuration file sample (heading and trailing whitespaces should be removed from name/value):
proc.sys.vm = 6 # ending comment
# comment followed by empty line

	yahoo.cops.vi= asasas asa

# there are no quotes for strings
# anything after '#' is treated as comment
# line endings are always '\n'
# everything is in pure english (simple ASCII), e.g. 1 byte == 1 character
# also count as correct empty values in lines like: 'a='
# if variable name contain whitespaces it is counted as incorrect, e.g. 'a b = c' not counted
*/

////////////////////////////////////
// forward declarations

/**
* struct cfg_stats_s - configuration file statistics.
*/
struct cfg_stats_s {
	int correct;								// count of correct lines in the file, where some variable is defined, e.g. count of variables definitions (without check for re-definitions)
	int vars_size;								// size of all variable name definitions in the file, e.g. 'x.y.z = 1' cost 5 bytes(x.y.z), 'proc.sys = uiy uuu ' costs 8 bytes(proc.sys)
	int values_size;							// size of all variable values definitions in the file, e.g. 'x.y.z = 1' cost 1 bytes(1), 'proc.sys = uiy uuu ' costs 7 bytes(uiy uuu)
	int lines;									// count of all lines in the file
	int comments;								// count of lines with comments in the file
};
/// create new config handle 
/// [in] path - path to configuration file
/// return config handle or NULL in case of error
void *cfg_open(const char *path);

/// destroy config handle and cleanup any resources associated with it
/// [in] cfg - configuration handle previously created with cfg_create
void cfg_close(void *cfg);

/// parse configuration file and calculate its stats
/// [in] cfg - config handle, previously created with cfg_open
/// [out] cs - structure filled with stats data upon successfull return
/// return zero if succeede or non-zero in case of error
int cfg_parse(void *cfg, struct cfg_stats_s *cs);

/// Process entry-point
int main(int argc, char *argv[])
{
	struct cfg_stats_s cs;						// config file stats
	void *cfg;									// config handle
	int i;										// iterator

	// check for command line arguments, we require configuration file path as one argument
	if (argc < 2) {
		printf("flespitest: invalid usage. Should be in form: ./flespitest <path>, where <path> is path of configuration file to calculate stats of\n");
		return -1;
	}

	// to to create config handle
	if ((cfg = cfg_open(argv[1])) == NULL) {
		printf("flespitest: cfg_open('%s') failed\n", argv[1]);
		return -1;
	}

	// ok, now parse it
	if (cfg_parse(cfg, &cs) == 0)
		// succeeded
		printf("flespitest: stats for '%s':\n correct: %d\n vars_size: %d\n values_size: %d\n lines: %d\n comments: %d\n", 
			argv[1], cs.correct, cs.vars_size, cs.values_size, cs.lines, cs.comments);
	else
		// failed
		printf("flespitest: cfg_parse failed\n");

	// test the performance
	for (i = 0; i < 10000; i++) {
		if (cfg_parse(cfg, &cs) != 0) {
			printf("flespitest: cfg_parse failed on %d iteration\n", i);
			break;
		}
	}
	
	// close config handle
	cfg_close(cfg);

	return 0;
}
////////////////////////////////////////////////////////
// PLESE REPLACE/FILL BELOW THIS LINES WITH YOUR VERSION

/// create new config handle 
/// [in] path - path to configuration file
/// return config handle or NULL in case of error
void *cfg_open(const char *path)
{
	FILE *fp;
	fp = fopen(path, "r"); //open text file for reading, the stream  is  positioned  at  the beginning of the file (see man fopen)
	if (fp == NULL) {
		return NULL;
	}
	return (void *)fp;
}

/// destroy config handle and cleanup any resources associated with it
/// [in] cfg - configuration handle previously created with cfg_create
void cfg_close(void *cfg)
{
	//function fclose is non-void, but prototype cfg_close - void.
	fclose(cfg);//upon  successful  completion 0 is returned.  Otherwise, EOF is returned and errno is set to indicate the error (from man)
}

/// parse configuration file and calculate its stats
/// [in] cfg - config handle, previously created with cfg_open
/// [out] cs - structure filled with stats data upon successfull return
/// return zero if succeede or non-zero in case of error
int cfg_parse(void *cfg, struct cfg_stats_s *cs)
{
	char c;
	int inword = 0; //when we are in the variable...
	int invalue = 0;//...and values
	int counted = 0;//counters for 
	int cnt_ch = 0;
	int cnt = 0;
	int isspace = 0;
	// memset(cs, 0, sizeof(struct cfg_stats_s));
	//reset struct to zero/null
	cs->correct = 0;
	cs->vars_size = 0;
	cs->values_size = 0;
	cs->lines = 0;
	cs->comments = 0;

	//return non-zero (-1) in case of error
	if (cfg == NULL)
		return -1;

	cs->lines++;
	enum states { //Enum construct declares states
		VARIABLE,
		VALUE,
		COMMENT,
		EMPTY
	} state = VARIABLE;
	//state machine that should handle most cases
	while ((c = fgetc(cfg)) != EOF) { //parsing each symbol
		switch (state) {
			case EMPTY: //incorrect lines
				switch(c) {
					case '\n': //each character '\n' is new line (for all states)
						cs->lines++;
						state = VARIABLE;
						cnt_ch = 0;
						break;
					case '#':
						state = COMMENT;
						break;
					default:
						break;
					}
				break;

			case VARIABLE: //count of variables definitions (without check for re-definitions)
				switch (c) {
					case '#': 
						state = COMMENT;
						break;
					case '\n':
						cs->lines++;
						cnt_ch = 0;
						break;
					case '=':
						if (!cnt_ch) {
							state = EMPTY;
						} else {
							cs->correct++; //count of correct lines in the file
							cs->vars_size += cnt_ch;
							state = VALUE;
							invalue = 1;//whitespaces should be removed from name/value
						}
						inword = 0;
						counted = 0;
						break;
					case ' ':
					case '\t':
						if(inword) { //
							inword = 0;
							counted++;
						}
						break;
					default:
						cnt_ch++;
						if(counted > 0) {
							state = EMPTY;
							inword = 0;
							counted = 0;
						} else {
							inword = 1;
						}
						break;
				}
				break;

			case VALUE:
				switch(c) {
					case '\n': 
						if (!isspace) { //if variable name contain whitespaces it is counted as incorrect, e.g. 'a b = c' not counted
							cs->values_size -= cnt;
						}
						cnt = 0;
						invalue = 1;
						cs->lines++;
						state = VARIABLE;
						cnt_ch = 0;
						break;
					case '#':
						if (!isspace) {
							cs->values_size -= cnt;
						}
						cnt = 0;
						invalue = 1;
						state = COMMENT;
						break;
					case '=':
						cs->values_size++;
						break;
					case ' ':
					case '\t':
						if(!invalue)//whitespaces should be removed from name/value
							cnt++;
						isspace = 1;
						break;
					default:
						invalue = 0;
						isspace = 0;
						cs->values_size++;
						cs->values_size += cnt;
						cnt = 0;
						break;
				}
				break;

			case COMMENT: //count of lines with comments in the file
				switch(c) {
					case '\n':
						cs->lines++;
						cs->comments++;
						state = VARIABLE;
						cnt_ch = 0;
					default:
						break;
					}
				break;
			default:
				break;
		}
	}
		return 0;
	}