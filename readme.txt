// 19/07/2015
// WebServer.h first used with Arduino and a wifi shield  to provide a web page intended to look like a central heating controller.
// Moved to Spark Core to over come the ram limitations.
// Debug output to USB Serail1.
// Gave up when Spark core was found to go 'out to lunch' for 80 seconds or so every three minutes.
// Resurrected more recently.  
// The web page is taken from an SD card with selective character replacements.
// Dallas temperature stuff added.
// Modified to use Tinker commands.  
// Modified to work with one input command and an output string.
// Modified to respond to local GET commands.
// Separately, Tinker modified to compile locally without need for special fonts.
// Separately, a 'tinker lite'  called 'Sparklite' written.
// HTML page written to use tinker style cloud commands or local GET commands
