/* COMP3361 Winter 2019 Lab 4 Sample Solution
 *
 * Process - execute memory trace file in the following format:
 *
 * Trace records contain multiple fields, separated by white space (blanks and 
 * tabs). Each line consists of a hexadecimal address, followed by an 
 * alphanumeric command name, followed by optional hexadecimal integer 
 * arguments to the command. The command name is case sensitive (all lower case).
 * 
 * Addresses are all in the range 0 to 3FFFFFF (26 bit values).
 * 
 * The trace file will contain the following record types. All numerical values 
 * (including counts) are hexadecimal (base 16), without a leading "0x". Any 
 * output should also use hexadecimal format for numeric data except where 
 * otherwise specified.
 * 
 * Memory Size Specification
 * address memsize
 * Set amount of memory to address bytes, rounded up to next multiple of page 
 * size. The first non-comment line of the file must be a mem command. 
 * Only a single mem command is allowed. Maximum value of address is 4000000.
 * 
 * Compare Bytes
 * addr1 cmp addr2 count
 * Compare count bytes starting at addr1 with count bytes starting at addr2.
 * If the values of any of the count bytes starting at addr1 are different from 
 * the values starting at addr2, write an error message to standard error for 
 * each difference with the addresses and the differing values  (all in 
 * hexadecimal). Follow the format shown in the sample output for the assignment.
 *  
 * Set Bytes
 * addr set values
 * Store values starting at addr; values is a list of byte values, separated 
 * by white space. 
 * 
 * Fill Range with Value
 * addr fill value count
 * Store count copies of value starting at addr.
 * 
 * Duplicate Range of Bytes
 * src_addr dup dest_addr count
 * Copy count bytes starting at src_addr into count bytes starting at dest_addr. 
 * The source and destination ranges will not overlap.
 * 
 * Print Bytes
 * addr print count
 * Print count bytes to standard output, starting at address addr. Write 
 * 16 bytes per line, with a single space before each value. Each line should 
 * start with the address of the first byte on that line, 7 hexadecimal digits 
 * with leading zeroes, followed by a colon. Print each byte as exactly 2 digits 
 * with a leading 0 for values less than 10 (hex). Separate the bytes with a 
 * single space. For example, to print the 24 bytes starting at fa700:
 * 
 * 00fa700: 00 12 f3 aa 00 00 00 a0 ff 0f e7 37 21 08 6e 00
 * 00fa710: 55 05 9a 9b 9c ba fa f0
 * 
 * Comment
 * * comment text
 * The # character in the first column means the remainder of the line should be 
 * treated as a comment. The command should be echoed to output in the same way 
 * as other commands, but should otherwise be ignored. Lines which are empty or 
 * all blank should also be treated as comments.
 * 
 * File:   Process.h
 * Author: Mike Goss <mikegoss@cs.du.edu>
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <MMU.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

class Process {
public:
  /**
   * Constructor - open trace file, initialize processing
   * 
   * @param file_name_ source of trace commands
   */
  Process(std::string file_name_);
  
  /**
   * Destructor - close trace file, clean up processing
   */
  virtual ~Process(void);

  // Other constructors, assignment: prevent copy and move
  Process(const Process &other) = delete;
  Process(Process &&other) = delete;
  Process operator=(const Process &other) = delete;
  Process operator=(Process &&other) = delete;
  
  /**
   * Run - read and process commands from trace file
   * 
   */
  void Exec(void);
  
private:
  // Trace file
  std::string file_name;
  std::fstream trace;
  long line_number;

  // Memory contents
  std::unique_ptr<mem::MMU> memory;
  
  /**
   * ParseCommand - parse a trace file command.
   *   Aborts program if invalid trace file.
   * 
   * @param line return the original command line
   * @param cmd return the command name
   * @param cmdArgs returns a vector of argument values
   * @return true if command parsed, false if end of file
   */
  bool ParseCommand(
      std::string &line, std::string &cmd, std::vector<uint32_t> &cmdArgs);
  
  /**
   * Command executors. Arguments are the same for each command.
   *   Form of the function is CmdX, where "X' is the command name, capitalized.
   * @param line original text of command line
   * @param cmd command, converted to all lower case
   * @param cmdArgs arguments to command
   */
  void CmdMemsize(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  void CmdCmp(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  void CmdSet(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  void CmdFill(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  void CmdDup(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  void CmdPrint(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
};

#endif /* PROCESS_H */

