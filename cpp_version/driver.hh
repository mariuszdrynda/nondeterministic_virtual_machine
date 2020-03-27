#ifndef DRIVER_HH
#define DRIVER_HH
#include <string>
#include "parser.tab.hh"

class driver{
public:
	int parse(const std::string& f); // Run the parser on file F.  Return 0 on success.
	void scan_begin();// Handling the scanner.
	void scan_end();
	std::string file; // The name of the file being parsed.
	yy::location location;// The token's location used by the scanner.
};
#endif // ! DRIVER_HH