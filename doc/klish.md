# The Klish user manual

## About ##

The Klish is a framework for implementing a CISCO-like CLI on a UNIX systems. It is configurable by XML files. The KLISH stands for Kommand Line Interface Shell. I know that "command" starts with "c" :) .

The Klish is a fork of [clish-0.7.3](http://sourceforge.net/projects/clish/) project <http://sourceforge.net/projects/clish/>. The original clish was developed by Graeme McKerrell. The Klish obtain some new features but it's compatible (as much as possible) with clish's XML configuration files. The author of Klish project is Serj Kalichev.

The main target for the Klish is a Linux platform. Additionally it can be build on FreeBSD, OpenBSD, Solaris. The QNX support is planned. The Klish project is written in C.

The Klish development is sponsored by ["Factor-TS"](http://www.factor-ts.ru/) company <http://www.factor-ts.ru/.>

### Klish related links

There are the Klish related links:

* GIT repository is <https://src.libcode.org/klish>
* Downloads is <http://klish.libcode.org/files>
* Issue tracker is <http://klish.libcode.org/issues/new>
* Klish discussion mailing list <http://groups.google.com/group/klish>
* Klish development mailing list <http://groups.oogle.com/group/klish-dev>
* News blog <http://klish-cli.blogspot.com>

### Features

The Klish incorporates all the features of clish. See the clish documentation on http://clish.sourceforge.net for details. Additionally Klish has some native features:

 * _[nested_views Namespaces or logically nested views]_. The tag [NAMESPACE] allows to import the command set from the specified view into another view. So these commands can be used within target view. It allows to create logically nested views. The further view in hierarchy can use commands of previous views. The behaviour is like a CISCO modes (there is the availability to use "configure"-mode commands from "config-if" mode).
 * _[nested_views Namespaces with prefix]_ support. The command set can be included into another view with the prefix. All included commands will obtain specified prefix when used from target view. These feature allow to implement CISCO-like "do show ..." commands.
 * _[optional_arguments Optional arguments]_ support. The command arguments can be optional. The [PARAM] tag supports "optional" option that specify whether parameter is optional.
 * _[subcommands Subcommands]_ support. The special type of PARAMs was implemented. It's a fixed word (the sequence of symbols with no spaces) that can be found among another arguments. It can be optional and mean a flag.
 * _[nested_params Nested parameters]_ support. The parameters can be nested. The child (nested) parameters follow the parent parameter. This feature use in parameter branching and with optional parameters. If optional parameter is entered then all the child parameters follow. If the optional parameter was not entered than the child parameters will not be used.
 * _[switch_subcommand Switch subcommands]_ support. The special type of subcommand allows to choose one argument of the list of possible arguments as a next positional parameter. So together with the [nested_params nested parameters] it implements the branching. The argument list can be non-linear.
 * _[cisco_config CISCO-like config]_ support. In some cases the CLI is rather useless without configuration commands storing. The new XML tag [CONFIG] was implemented to support interaction beetween Klish and some external (or internal) mechanism to store some commands sequence i.e. CISCO-like configuration. On each succesfully executed command the Klish can execute special callback function that get current command information and can communicate to external tool to store commands or use the internal mechanisms.
 * _[konfd Configuration daemon]_. The configuration daemon [konfd] can store the current CISCO-like configuration information (running-config). Any Klish or another process can communicate to [konfd] via socket. There is special string-oriented protocol to set new entries to the running-config, remove existent entries or get current config state.
 * _[CLISH_VIEW The initial view redefinition]_. User can define CLISH_VIEW environment variable to set initial view instead of the initial view from STARTUP tag.
 * _[Klish_examples The Klish specific XML examples]_. The Klish source tree contain the Klish specific XML examples that show basic CISCO-like interface to configure network interfaces and routing in Linux system.
 * _[sequence The ordered sequences]_ support in user configuration. In some cases the ordered numerated lists is needed. The example is a CISCO-like access lists in which the order of entries is important. The entries can be addressed by the line number.
 * _[internal_variables The automatic internal variables]_. For each command the Klish engine generates the automatic variables that can be used the same way as a variables origin from PARAM tags. These are current command line (${__cmd}), the whole entered line (${__line}) etc.
 * _[builtin_functions The Klish specific builtin functions]_. The clish contain a set of builtin functions (that don't need a scripting within ACTION tag). The additional Klish specific builtin functions is available.
 * _[conditional_param The conditional parameters]_ support. The [PARAM parameter] can be dynamically enabled or disabled depending on the condition. The condition have the syntax same as standard /bin/test utility. So the [PARAM parameter] visibility can depend on the previous [PARAM parameters] values.
 * _[locking_mechanism The locking mechanism]_. The locking mechanism allows to execute several instances of clish utility (or another programs based on libclish library) simultaneously without conflicts.
 * _[atomic_action The atomic actions]_ support. The [ACTION] script can be non-interruptable for the user. It's a default behaviour.
 * _[shebang The choosing of the scripting language]_ is supported. The scripting language for the [ACTION] script execution can be customized.
 * _[command_alias The command aliases]_ are supported. The [COMMAND command] can have the aliases. The resulting alias is equal to the original command. To find out what name (original or alias) was used the `${__cmd}` internal variable can be analyzed.
 * _[buildroot The buildroot contrib files]_. The Klish source tree contain the contrib files for the [http://www.buildroot.net buildroot] to be embedded into it as an additional package.
 * _[utf8 The UTF-8 encoding support]_. The clish utility can autodetect if current locale use UTF-8 encoding or 8-bit encoding.
 * _[xml_backend XML backends]_. The Klish engine supports a several XML backends. That backends parses Klish's XML configuration files.
 * _[hotkeys Programmable hotkeys]_. The programmable hotkeys were implemented.
----

### Utilities

The utilities reference:

  * [clish](#utility_clish) - The CLI utility.
  * konfd konfd - The daemon to store configuration.
  * utility_konf konf - The utility to communicate to konfd daemon from shell.
  * sigexec - The utility to start daemons from non-interruptable ACTION scripts.

----
== XML tags/parameters ==

The following list represents the Klish native XML tags of early-known tags with Klish native options:

  * [NAMESPACE]
  * [CONFIG]
  * [PARAM]
  * [VIEW]
  * [COMMAND]
  * [ACTION]
  * [STARTUP]
  * [HOTKEY]

----
== HOWTO ==

See the [HOWTO] to find out some Klish related questions.

----
(C) _Serj Kalichev <serj.kalichev(`_at_`)gmail.com>, 2010_





# A modular framework for implementing a CISCO-like CLI on a UNIX system.

# CLISH

A modular framework for implementing a CISCO-like CLI on a UNIX system. Arbitary command menus and actions can be defined using XML files. This software handles the user interaction, and forks the appropriate system commands to perform any actions.

The clish was developed by [Graeme McKerrell](gmckerrell).

- http://sourceforge.net/projects/clish/
- http://clish.sourceforge.net# Graeme McKerrell

# Graeme !McKerrell

The author of the clish project.
http://sourceforge.net/projects/clish/








## Features

### The atomic actions
The [script can be interruptable or non-interruptable (atomic). It's often usefull together with the [locking_mechanism locking mechanism](ACTION]). 

To make the action atomic the [script must be non-interruptable for the user. The following signals can be blocked while [ACTION](ACTION]) script execution:

* SIGINT
* SIGQUIT
* SIGHUP

So the user can't use Ctrl^C to terminate current process. The signal blocking is default behaviour for the [execution now. The signal blocking is implemented by sigprocmask() function. The blocked mask will be inherited by all executed processes.

To make action interruptable the [COMMAND](ACTION]) tag must contain interrupt="true" field. For example the 'ping' command may be interruptable.

```
<COMMAND name="ping"
	help="Send messages to network hosts"
	interrupt="true">
	...
</COMMAND>
```

See the [field 'interrupt'. The atomic actions is available since SVN revision 347 or Klish-1.4.0. The SIGHUP signal is blocked since Klish-1.5.6 and Klish-1.6.1.

### Daemon execution

Note when the [ACTION](COMMAND]) is non-interruptable the daemon execution within script is not safe enough because many services (daemons) use SIGHUP (or some another signals) for its work. For example the SIGHUP is signal to reread configuration files often. But when the [is non-interruptable the service will inherit signal mask from script so some signals will be masked and service will never get these signals. The right way to start services is to use special utility [sigexec](ACTION]). This utility will unmask all signals and then execute command specified in its command line:

```
# sigexec /etc/init.d/vsftpd start
```

Use [sigexec] utility everytime you start service and the service environment will be more suitable.

### The builtin functions.

The original clish contain a set of builtin functions that don't need a scripting within ACTION tag. The example of of using builtin function from original clish. This command closes the current clish session.

```
<COMMAND name="logout"
	help="Logout of the current CLI session">
	<ACTION builtin="clish_close"/>
</COMMAND>
```

The additional Klish specific builtin functions is available.

#### clish_nested_up

The Klish supports the [nested views. When user moves deeper in the [VIEW](nested_views])'s hierarchy (the 'depth' of [is increasing) the engine create entries in the "nesting" stack to save previous [VIEW](VIEW]s) and its state. The clish_nested_up function make 'pop' stack operation so it restores previous [and its state. If the current depth is 0 then the clish_nested_up function will be an analog of clish_close builtin function and will close the current Klish session.

The feature is available starting with Klish-1.2.1.

#### clish_nop

The NOP command. It does nothing. It's usefull for commands like comment. The CISCO uses "!" as a comment. If the command has no [ACTION](VIEW]) tag at all then this command is unfinished so the "Enter" can't be pressed. Use clish_nop for empty commands. Don't use `<ACTION></ACTION>` because it's slower.

The feature is available starting with Klish-1.4.2.

### CISCO-like config support.

In some cases the CLI is rather useless without configuration commands storing. The new XML tag [was implemented to support interaction beetween Klish and some external (or internal) mechanism to store some commands sequence i.e. CISCO-like configuration. On each succesfully executed command the Klish can execute special callback function that get current command information and can communicate to external tool to store commands or the internal mechanisms can be used for config storing. 

The default tool to store configuration is [konfd](CONFIG]) daemon accessible over socket interface.

The [tag is used to make Klish to store the current command or execute other actions (remove old entries, dump entries to file) on config. See [CONFIG](CONFIG]) page for details about tag syntax.

### The command aliases

The [command](COMMAND) can have the aliases. The resulting alias is equal to the original command except the name of command, help text and the value of internal variable `${__cmd}`.

To find out what name (original or alias) was typed by the user the `${__cmd}` internal variable can be analyzed. The `${__orig_cmd}` variable contain the name of the original command (the target of alias). See [internal variables](internal_variables) page for details about internal variables. The `${__cmd}` can be analyzed within [and within [conditional_param 'test' field](ACTION]) of [tag.

The 'ref' field of [COMMAND](PARAM]) tag is used to create alias of command. The following example creates the alias (named "info") for the original command "show running-config" from the view "view1":

```
<COMMAND name="info" ref="show running-config@view1" help="Alias for the show running-config command"/>
```

The following example creates alias (named "conf") for the command "info" from the current view (the view of both "conf" and "info" commands):

```
<COMMAND name="conf" ref="info" help="Alias for the info command"/>
```    

### The conditional parameters.

The [parameter](PARAM) can be dynamically enabled or disabled depending on the condition. The condition have the syntax same as standard /bin/test utility. So the [parameter](PARAM) visibility can depend on the previous [parameters](PARAM) values.

The [tag has the optional 'test' field. The 'test' field contains the expression to evaluate. If the expression is evaluated to false the PARAM does not appear at CLI shell and the ${param_name} variable will be empty. If the [PARAM](PARAM]) is disabled you can consider it as the parameter is not exists. The [internal variables](internal_variables) and previous [parameters](PARAM) values can be used within expression. The following syntax is used to construct expression (from OpenBSD man pages):

     -n string
             True if the length of string is nonzero.
     -z string
             True if the length of string is zero.
     s1 = s2
             True if the strings s1 and s2 are identical.
     s1 != s2
             True if the strings s1 and s2 are not identical.
     s1 < s2
             True if string s1 comes before s2 based on the ASCII value of
             their characters.
     s1 > s2
             True if string s1 comes after s2 based on the ASCII value of
             their characters.
     s1      True if s1 is not the null string.
     n1 -eq n2
             True if the integers n1 and n2 are algebraically equal.
     n1 -ne n2
             True if the integers n1 and n2 are not algebraically equal.
     n1 -gt n2
             True if the integer n1 is algebraically greater than the integer
             n2.
     n1 -ge n2
             True if the integer n1 is algebraically greater than or equal to
             the integer n2.
     n1 -lt n2
             True if the integer n1 is algebraically less than the integer n2.
     n1 -le n2
             True if the integer n1 is algebraically less than or equal to the
             integer n2.
     These primaries can be combined with the following operators.  The -a
     operator has higher precedence than the -o operator.
     ! expression
             True if expression is false.
     expression1 -a expression2
             True if both expression1 and expression2 are true.
     expression1 -o expression2
             True if either expression1 or expression2 are true.
     ( expression )
             True if expression is true.

The example demonstrate the parameter "size" that will be enabled if "proto" is "ip" or "ipv6" and will be disabled if proto is "arp".

```
<COMMAND name="ping"
	help="Send messages to network hosts">

	<PARAM name="proto"
		help="Protocol to use for the ping"
		optional="true"
		mode="switch"
		ptype="SUBCOMMAND">

		<PARAM name="ip"
			help="Send ICMP IPv4 messages to network hosts (default)"
			mode="subcommand"
			ptype="SUBCOMMAND"/>
		<PARAM name="ipv6"
			help="Send ICMP IPv6 messages to network hosts"
			mode="subcommand"
			ptype="SUBCOMMAND"/>
		<PARAM name="arp"
			help="Send ARP requests to a neighbour host"
			mode="subcommand"
			ptype="SUBCOMMAND"/>

	</PARAM>
  
	...
    
	<PARAM name="size"
		test='"${proto}"!="arp"'
		help="Packet size"
		optional="true"
		mode="subcommand"
		ptype="SUBCOMMAND">

		<PARAM name="psize"
			help="Number of data bytes to send"
			ptype="UINT"/>

	</PARAM>
    
	...
    
</COMMAND>
```
                    
### The programmable hotkeys

The key combinations (hotkeys) can be programmed to execute specified actions. Use [tag to define hotkey and its action.

All [VIEW](HOTKEY])s (including global implicit [can contain [HOTKEY](VIEW])) tag to define hotkey for this [If the nested [VIEW](VIEW].)s are used the Klish engine will search for hotkey definition in the current [then in the upper [VIEW](VIEW],) and so on. So you can define [within global [VIEW](HOTKEY]) and all other [will share this definition.

See the example:
38c058073974034f0beaae53eec90be5

This example defines two hotkeys. The "`Ctrl^Z`" for exit. It is defined in the global [VIEW](VIEW]s) and will act in the all [(in a case it will not be redefined in the nested [VIEW](VIEW]s)s).

The second hotkey is "`Ctrl^@`". Both "`Ctrl^@`" and "`Ctrl^spacebar`" combinations give this key code. This hotkey will execute "show" command in the "enable-view" [and all its subviews but the "do show" command in "configure-view" [VIEW](VIEW]) and all its subviews.

#### Possible keys

Some keys has predefined hardcoded behaviour. If key has a predefined behaviour it can't be redefined (used in [tag) now.

<table>
  <tr><td>*Code*</td><td>*Id*</td><td>*Key*</td><td>*Action*</td><td>*Comment*</td></tr>
  <tr><td>0</td><td>NUL</td><td>^@</td><td></td><td>Null character</td></tr>
  <tr><td>1</td><td>SOH</td><td>^A</td><td>Home</td><td>Start of heading, = console interrupt</td></tr>
  <tr><td>2</td><td>STX</td><td>^B</td><td></td><td>Start of text, maintenance mode on HP console</td></tr>
  <tr><td>3</td><td>ETX</td><td>^C</td><td>Break</td><td>End of text</td></tr>
  <tr><td>4</td><td>EOT</td><td>^D</td><td>Delete</td><td>End of transmission, not the same as ETB</td></tr>
  <tr><td>5</td><td>ENQ</td><td>^E</td><td>End</td><td>Enquiry, goes with ACK; old HP flow control</td></tr>
  <tr><td>6</td><td>ACK</td><td>^F</td><td></td><td>Acknowledge, clears ENQ logon hand</td></tr>
  <tr><td>7</td><td>BEL</td><td>^G</td><td></td><td>Bell, rings the bell...</td></tr>
  <tr><td>8</td><td>BS</td><td>^H</td><td>Backspace</td><td>Backspace, works on HP terminals/computers</td></tr>
  <tr><td>9</td><td>HT</td><td>^I</td><td>Tab</td><td>Horizontal tab, move to next tab stop</td></tr>
  <tr><td>10</td><td>LF</td><td>^J</td><td>Enter</td><td>Line Feed</td></tr>
  <tr><td>11</td><td>VT</td><td>^K</td><td>Kill line</td><td>Vertical tab</td></tr>
  <tr><td>12</td><td>FF</td><td>^L</td><td>Clear screen</td><td>Form Feed, page eject</td></tr>
  <tr><td>13</td><td>CR</td><td>^M</td><td>Enter</td><td>Carriage Return</td></tr>
  <tr><td>14</td><td>SO</td><td>^N</td><td></td><td>Shift Out, alternate character set</td></tr>
  <tr><td>15</td><td>SI</td><td>^O</td><td></td><td>Shift In, resume defaultn character set</td></tr>
  <tr><td>16</td><td>DLE</td><td>^P</td><td></td><td>Data link escape</td></tr>
  <tr><td>17</td><td>DC1</td><td>^Q</td><td></td><td>XON, with XOFF to pause listings; "okay to send".</td></tr>
  <tr><td>18</td><td>DC2</td><td>^R</td><td></td><td>Device control 2, block-mode flow control</td></tr>
  <tr><td>19</td><td>DC3</td><td>^S</td><td></td><td>XOFF, with XON is TERM=18 flow control</td></tr>
  <tr><td>20</td><td>DC4</td><td>^T</td><td></td><td>Device control 4</td></tr>
  <tr><td>21</td><td>NAK</td><td>^U</td><td>Erase line</td><td>Negative acknowledge</td></tr>
  <tr><td>22</td><td>SYN</td><td>^V</td><td></td><td>Synchronous idle</td></tr>
  <tr><td>23</td><td>ETB</td><td>^W</td><td>Erase word</td><td>End transmission block, not the same as EOT</td></tr>
  <tr><td>24</td><td>CAN</td><td>^X</td><td></td><td>Cancel line, MPE echoes !!!</td></tr>
  <tr><td>25</td><td>EM</td><td>^Y</td><td>Yank</td><td>End of medium, Control-Y interrupt</td></tr>
  <tr><td>26</td><td>SUB</td><td>^Z</td><td></td><td>Substitute</td></tr>
  <tr><td>27</td><td>ESC</td><td>^[</td><td>Escape</td><td>Escape, next character is not echoed</td></tr>
  <tr><td>28</td><td>FS</td><td>^\</td><td></td><td>File separator</td></tr>
  <tr><td>29</td><td>GS</td><td>^](HOTKEY])</td><td></td><td>Group separator</td></tr>
  <tr><td>30</td><td>RS</td><td>^^</td><td></td><td>Record separator, block-mode terminator</td></tr>
  <tr><td>31</td><td>US</td><td>`^_`</td><td></td><td>Unit separator</td></tr>
</table>

### The automatic internal variables.

For each command the Klish engine generates the automatic variables that can be used the same way as a variables origin from [tags. To specify these variables use ${`<name>`} syntax. The variables will be expanded before execution of ACTION, before using some tag's fields that is dynamic and allow the using of variables. The example of such field is [CONFIG](PARAM])'s 'pattern'.

#### `${__cmd}`

The `${__cmd}` contain the name of the current command.

#### `${__orig_cmd}`

The `${__orig_cmd}` contain the name of the original command if the current command is [alias](command_alias). If the current command is not [alias](command_alias) the `${__orig_cmd}` is equal to the `${__cmd}`.

#### `${__full_cmd}`

The `${__full_cmd}` is the entered name of command. When the using of command is simple the `${__full_cmd}` will be equal to `${__cmd}` variable. But when the command was imported from another [using the [NAMESPACE](VIEW]) tag with specified 'prefix' field, the `${__full_cmd}` will contain the full name of command "`<prefix>` `<command>`".

For example the "show" command is defined in the ["enable-view". The current [VIEW](VIEW]) is "configure-view". The ["enable-view" is imported into "configure-view" using [NAMESPACE](VIEW]) tag with field prefix="do". If user enters "do show" in command line than the `${__cmd}`="show" but `${__full_cmd}`="do show".

#### `${__params}`

The `${__params}` contain all the entered command parameters. It's equal to line entered by user without command name. If one of the parameters contain spaces than this parameter will be quoted within `${__params}` line.

#### `${__line}`

The `${__line}` is equal to "`${__cmd}` `${__params}`".

#### `${__full_line}`

The `${__full_line}` is equal to "`${__full_cmd}` `${__params}`".

#### `${__prefix}`

If the current command is imported from another [using [NAMESPACE](VIEW]) tag with specified prefix than the `${__prefix}` will contain the actually entered prefix (the prefix definition can be a regexp).

#### `${__prefix<num>}`

If the current command has several prefixes in a case of nested imports than the `${__prefix<num>}` will contain the actually entered prefix with number `<num>` in a line. The `${__prefix0}` is equal to `${__prefix}`.

#### `${__cur_depth}`

The `${__cur_depth}` contain the current depth of [nested views](nested_views). Note it's not a depth of current command's [but a depth of current [VIEW](VIEW]). These values is not equal when the command is imported from the another [## `${__cur_pwd}`

The `${__cur_pwd}` contain the "path" to the current [VIEW](VIEW].). The views can be [nested](nested_views) and the commands that lead to changing view to the current [is the current "path". These commands is quoted and delimeted by the space. Usually the "path" is used while communication to the [konfd](VIEW]) daemon. It allows to find out the position and depth of current command in the user config.

#### `${__interactive}`

The `${__interactive}` can be used to find out if the [clish](utility_clish) session is interactive (`${__interactive}` is equal to "1") or non-interactive (`${__interactive}` is equal to "0"). By default the session is interactive. But the [clish utility](utility_clish) can be executed with the "--background" option to make session non-interactive.

#### `${__isatty}`

The `${__isatty}` variable indicates if command was entered manually (using interactive tty) or it come from file or piped stdin. The value will be equal to "1" if command was entered manually and user have interactive tty. The value will be equal to "0" if command came from file.

The variable is available since SVN revision #546 or Klish-1.5.2 release.


#### `${__width}`

The current terminal width (columns).

#### `${__height}`

The current terminal height (rows).

#### `${__watchdog_timeout}`

The current watchdog timeout. When the watchdog is inactive the value is "0".

#### The automatic variables using example

The example shows the using of automatic internal variable `${__line}` in CONFIG's pattern field.

```
<COMMAND name="interface ethernet"
	help="Ethernet IEEE 802.3"
	view="configure-if-view"
	viewid="iface=eth${iface_num}">

	<PARAM name="iface_num"
		help="Ethernet interface number"
		ptype="IFACE_NUM"/>
	<CONFIG priority="0x2001" pattern="^${__line}$"/>

</COMMAND>
```
            
### The locking mechanism.

The locking mechanism allows to execute several instances of clish utility (or another programs based on libclish library) simultaneously without conflicts. It's often usefull together with the [atomic actions](atomic_action).

Before the command execution the Klish engine try to get lock. The implementation of lock is a flock() call on the /tmp/clish.lock file. If process can't to get lock it repeats the attempts to get lock for the several times with the short sleep() beetween attempts. The fail means that someone got the lock earlier. If the process got lock it execute the command's ACTION and than unlock the file.

Some commands don't need the locking mechanism. The example of such command is 'ping'. It is independent command. The several 'ping's can be executed simultaneously. But the commands that can change the user config or some system settings need the locking mechanism. So the locking mechanism can be enabled/disabled on the per command basis. The [COMMAND] tag has the 'lock' field that can be 'true' or 'false'. It's 'true' but default. But if it's 'false' the Klish engine will not try to get lock before this command execution.

The example of lockless 'ping' command:

```
<COMMAND name="ping"
	help="Send messages to network hosts"
	lock="false">
	...
</COMMAND>
```

Sometimes it's needed to disable locking mechanism for the whole clish session. It's usefull if the clish utility is used from another clish instance from the command's ACTION. The command already got the lock so the nested clish can't get the lock. The nested clish can be executed with "-l" ( or "--lockless") option to disable locking mechanism.

The example of nested clish execution:

```
<COMMAND name="nested"
	help="This command use nested clish utility">
	<ACTION>
		echo "ping www.google.com" | clish --lockless
	</ACTION>
</COMMAND>
```

### Nested parameters and parameter branching.

The parameters can be nested i.e. [can contain another sub-PARAMs. 

The sub-parameters will be taken into account then the parent parameter is set. If the parent parameter is not set cause it's [optional_arguments optional](PARAM]) or then engine choose another execution way while [branching](switch_subcommand) the sub-paremeters will not be taken into account and the corresponding variables will be unset. After analyzing sub-parameters engine will continue the normal flow and will analyze the next parameter after parent parameter (which contain sub-parameters).

There is a good example of using nested parameters in [optional parameters](optional_arguments) page. See the Klish native variant. Another good example of nesting you can find in [switch subcommand](switch_subcommand) page.

### The namespaces or logically nested views.

The tag [allows to import the command set from the specified view into another view. So these commands can be used within target view. It allows to create logically nested views. The further view in hierarchy can use commands of previous views. The behaviour is like a CISCO modes (there is the availability to use "configure"-mode commands from "config-if" mode). See the [NAMESPACE](NAMESPACE]) for the tag description.

#### Logically nested views

The following code demonstrates the using of command set import. Assume the current view is "view2". The command "com1" will be available although it belogs to "view1". Additionally the help and completion for commands from "view1" is enabled while import.

```
<VIEW name="view1" prompt="(view1)# ">
    
	<COMMAND name="com1" help="Command 1">
		<ACTION>echo "com1"</ACTION>
	</COMMAND>
    
</VIEW>
    
    
<VIEW name="view2" prompt="(view2)# ">
    
	<NAMESPACE ref="view1"
		help="true"
		completion="true"/>
   
	<COMMAND name="com2" help="Command 2">
		<ACTION>echo "com2"</ACTION>
	</COMMAND>
    
	<COMMAND name="exit" help="Exit">
		<ACTION builtin="clish_close"/>
	</COMMAND>
    
</VIEW>
```

#### Namespaces with prefix

The following code do the same thing as a previous example but the commands from "view1" is available with **prefix** "do". Note the new command within "view2" named "do". This command is necessary for Klish to resolve any command with this prefix.

```
	<VIEW name="view1" prompt="(view1)# ">
    
	<COMMAND name="com1" help="Command 1">
		<ACTION>echo "com1"</ACTION>
	</COMMAND>
    
</VIEW>
    
    
<VIEW name="view2" prompt="(view2)# ">
    
	<NAMESPACE ref="view1"
		prefix="do"
		help="true"
		completion="true"/>
    
	<COMMAND name="do" help="Import prefix"/>
    
	<COMMAND name="com2" help="Command 2">
		<ACTION>echo "com2"</ACTION>
	</COMMAND>
    
	<COMMAND name="exit" help="Exit">
		<ACTION builtin="clish_close"/>
	</COMMAND>
    
</VIEW>
```

#### Restore the command context

When the hierarchy of nested [views](VIEW) is built the lower views inherit the [commands](COMMAND) from the higher [views](VIEW) often. It's true for the CISCO-like configuration mode. See the [Klish XML examples](Klish_examples) in the source tree. For example if the current view is the "configure-if-view" (the nested view to define network interface settings) it's not necessary to obviously "exit" from the current view to the higher level view ("configure-view") to use commands from this higher level view. You can execute these commands directly if the same commands was not redefined in the current view. This feature is used to process the plain [config](cisco_config) files. 

By default the current view will not be changed. It's good for the information commands like the "do show running-config" that will not get to the [user config](cisco_config). But it's not good for the configuration commands. The such commands use the current context (the view, depth and viewid) while execution. So it's necessary to set the command's native context before its execution. The direct call of the higher level command is equal to "exit" from the nested view to the higher level view and then the execution of the specified command. To implement such behaviour without obvious "exit" the 'restore' field of the [tag can be used. 

All the commands of the [VIEW](VIEW]) with specified 'restore' field will restore its context when executed from another view which use [mechanism. See the [VIEW](NAMESPACE]) tag description for the details. The command can restore only its native view (set it as the current view) if the restore="view" but it's not usefull often. Because this method can't restore viewid and the "configure-view" can include (using [commands from the other [VIEW](NAMESPACE]))s with the same depth. The more usefull method for the hierarchy of the nested views is restore="depth". The Klish engine will find out the depth of command's native view. Then it will search for this depth in the current nested views stack. The current view will be set to the saved view from the stack with the depth equal to command's depth. Additionally the saved context (the viewid) will be restored from the stack.

The typical "configure-view" has the restore="depth" field:

```
<VIEW name="configure-view"
	prompt="${SYSTEM_NAME}(config)# "
	restore="depth">
    
	....
    
	<COMMAND name="interface"
		help="Select an interface to configure"/>
    
	<COMMAND name="interface ethernet"
		help="Ethernet IEEE 802.3"
		view="configure-if-view"
		viewid="iface=eth${iface_num}">
		<PARAM name="iface_num"
			help="Ethernet interface number"
			ptype="IFACE_NUM"/>
		<CONFIG priority="0x2001" pattern="^${__line}$"/>
	</COMMAND>
    
	....
    
</VIEW>
    
<VIEW name="configure-if-view"
	prompt="${SYSTEM_NAME}(config-if-${iface})# "
	depth="1">
    
	...
    
	</VIEW>
```	
	
### The optional arguments

The command arguments can be optional. The [tag supports "optional" parameter that specify whether parameter is optional. It can be a sequence of optional parameters. The order of optional parameters define the order to validate values. If the value was succesfully validated by optional parameter the next optional parameters will not validate this value. Each parameter can be specified only once. See the [PARAM](PARAM]) for the tag description.

The following code creates three optional arguments and the mandatory one:

```
<PTYPE name="SUBCOMMAND"
	pattern="[^\]+"
	help="String"/>
    
<COMMAND name="com2" help="Command 2">
    
	<PARAM name="flag"
		help="option -c"
		ptype="SUBCOMMAND"
		mode="subcommand"
		optional="true"/>
   
	<PARAM name="-c"
		help="option -c"
		ptype="SUBCOMMAND"
		mode="subcommand"
		optional="true"/>
   
	<PARAM name="p_int"
		help="optional uint param"
		ptype="UINT"
		optional="true"/>
    
	<PARAM name="mandatory"
		help="mandatory uint param"
		ptype="SUBCOMMAND"
		mode="subcommand"/>
    
	<ACTION>
		if test "x${flag}" != "x"; then echo "flag=${flag}"; fi
		if test "x${-c}" != "x"; then echo "-c=${-c}"; fi
		if test "x${p_int}" != "x"; then echo "p_int=${p_int}"; fi
		echo "${mandatory}"
	</ACTION>

</COMMAND>
```

If optional parameters has not been entered then corresponding variable will not be set. The optional parameters can be used in arbitrary order.

#### Ordered optional parameters

In previous example all three optional parameters can be used in arbitrary order i.e. you can enter the "-c" parameter first and "flag" parameter later or any other order. The "order" field of [tag makes the sequence of optional parameters ordered.

```
<PARAM name="flag"
	help="option -c"
	ptype="SUBCOMMAND"
	mode="subcommand"
	optional="true"/>
    
<PARAM name="-c"
	help="option -c"
	ptype="SUBCOMMAND"
	mode="subcommand"
	optional="true"
	order="true"/>
    
<PARAM name="p_int"
	help="optional uint param"
	ptype="UINT"
	optional="true"/>
```

Notice the order="true" field within "-c" subcommand definition. Now the "flag" optional parameter can't be entered if "-c" is already entered. So the parameter with order field cut off all previously declared optional parameters.

#### The clish compatibility

The [clish](PARAM]) has the optional parameters support too but there is a differencies. The "prefix" [option definition means that parameter is optional and the prefix must be followed by argument with "ptype" specified in the same [PARAM](PARAM]). So the parameter without prefix cannot be optional.

The Klish emulates clish behaviour when the "prefix" option is defined. The following two [is equivalent.

- The Klish native variant:
ed6308cd3d5b352612db98dc2fbb8e37

- The clish compatible variant:
d9c67481e57f9116169fdf8af8ad18a0

The internal representation of these parameters is the same. The Klish native variant can show the internal representation. It use [nested_params nested parameters](PARAM]s) mechanism. Actually the clish variant use internal ptype "internal_SUBCOMMAND" for auto-generated optional subcommand named "-c". It cannot use "SUBCOMMAND" ptype bacause it can be undefined. The "internal_SUBCOMMAND" ptype has pattern="`[^\]+`" and help="Option".

The clish variant seems to be shorter but it doesn't work if you need several sub-parameters or if you don't need any sub-parameters at all but only the flag.

### The ordered sequences

In some cases the ordered numerated lists is needed. The example is a CISCO-like access lists in which the order of entries is important. The entries can be addressed by the line number.

### The choosing of the scripting language


The scripting language for the [script execution can be customized. Additionally the user can define the default scripting language for the whole session.

The [http://en.wikipedia.org/wiki/Shebang_%28Unix%29 shebang](ACTION]) can be specified for the [script execution. By default the "/bin/sh" is used. To customize shebang the 'shebang' field of the [ACTION](ACTION]) tag is used:

```
<COMMAND ...>
	...
	<ACTION shebang="/usr/bin/perl -w">
		print "Hello world\n";
	</ACTION>
</COMMAND>
```

To define the default shebang for the whole session the 'default_shebang' field of the [STARTUP] tag is used:

```
<STARTUP default_shebang="/bin/bash" ... />
```  
    
### The subcommands description

The special type of [was implemented. It's a fixed word (the sequence of symbols with no spaces) that can be found among another arguments. The subcommand is identified by its name (or "value" field if specified) and can be used as optional flag or for the branching. If the subcommand was used then the value of parameter is its name ("value" field if specified). The value of parameter is undefined if parameter is optional and was not used. See the [PARAM](PARAM]) for the tag description.

The following example shows typical subcommand definition.

```
<PTYPE name="SUBCOMMAND"
	pattern="[^\]+"
	help="String"/>

...
    
<PARAM name="-c"
	help="option"
	ptype="SUBCOMMAND"
	mode="subcommand"
	optional="true"/>
```

The [tag contain "mode" option. This option must be "subcommand" for subcommands. See the "mode" option description in [PARAM](PARAM]). The "ptype" may be arbitrary. The engine will validate the name of [using specified "ptype".

I think there is no reason to use subcommands without optional="true" or without [switch_subcommand branching](PARAM]). See the [optional arguments](optional_arguments) and [switch subcommands](switch_subcommand) for additional information.

#### Subcommand duplication

The displayable subcommand name can be duplicated by the "value" field. For example the user can use two subcommands "host":

```
<PARAM name="host1"
	value="host"
	help="src host"
	ptype="SUBCOMMAND"
	mode="subcommand">

	<PARAM name="ip_src"
		help="src host ip"
		ptype="IP_ADDR"/>

</PARAM>
    
<PARAM name="host2"
	value="host"
	help="dst host"
	ptype="SUBCOMMAND">

	<PARAM name="ip_dst"
		help="dst host ip"
		ptype="IP_ADDR"/>

</PARAM>
```

The internal [variable names will be "host1" and "host2" but the displayable names is "host" for the both parameters. The "value" field forces the mode of [PARAM](PARAM]'s) to "subcommand". It have no meaning for another modes. If the "value" field is not specified the internal variable name and displayable name is the same and the "name" field is used.

The feature is available since version 1.2.0.

### The switch subcommand.

The special type of [was implemented. The switch subcommand is a container that allow to choose the only one of its sub-parameters for further analyzing.

The following example shows the command with switch subcommand:

```
<COMMAND name="com" help="Command">
   
	<PARAM name="choice"
		help="Switch subcommand"
		ptype="STRING"
		mode="switch">
    
		<PARAM name="one"
			help="one subcommand"
			ptype="SUBCOMMAND"
			mode="subcommand"/>
   
		<PARAM name="two"
			help="two subcommand"
			ptype="SUBCOMMAND"
			mode="subcommand">
    
			<PARAM name="nint"
				help="nested int"
				ptype="UINT"/>
    
		</PARAM>
    
	</PARAM>
    
	<PARAM name="mandatory"
		help="mandatory uint param"
		ptype="UINT"/>
    
		<ACTION>
			echo "Choice is ${choice}"
			echo "one is ${one}"
			echo "two is ${two}"
			echo "nint is ${nint}"
		</ACTION>
    
</COMMAND>
```

To define the switch subcommand the [PARAM](PARAM])'s option "mode" must be set to "switch". The "ptype" of switch subcommand define PTYPE for sub-parameters names validation.

When the user types "com" he must choose the one of two variants: "one" or "two". So user choose the branch for futher command line parsing. The switch subcommand named "choice" will be set to the name of choosen sub-parameter so you can analyze this value later. The variable with name of choosen sub-parameter will be set to its own name. The variables with names of sub-parameters that were not choosen will be unset.

Suppose we choose the "two" sub-parameter. This [subcommand](subcommands) contain [nested parameter](nested_params) named "nint". The next command line argument will parsed for the "nint" parameter. Then the engine will return to the normal flow and will analyze "mandatory" parameter. The "one" variable will be unset and the "choice" value will be "two". So it implements branching. The "one" branch was not used at all.

### The UTF-8 support

The Klish ([clish](utility_clish) console utility) supports UTF-8 and 8-bit encodings.

The [clish utility](utility_clish) autodetects the current encoding using locale information. So the console input behaviour differs for UTF-8 and traditional 8-bit encodings.

If the locale is broken user can force using of the UTF-8 encoding by "-u" (--utf8) option on the clish's utility command line. The "-8" (--8bit) option is used to force 8-bit encoding.

The UTF-8 support is available since SVN revision 345 or Klish-1.4.0.







## Tags

### ACTION


The ACTION tag defines the script to execute. This document describes Klish native options only. See the clish documentation for the other ACTION options.


#### [shebang]
Defines the scripting language (the binary file) to use for the ACTION script execution.

Default is the shebang defined within [STARTUP](shebang]`) tag using 'default_shebang' field. If the 'default_sheband' is undefined the "/bin/sh" is used.




### COMMAND

The COMMAND tag defines the command. This document describes Klish native options only. See the [documentation for  the other COMMAND options. See the [locking_mechanism locking mechanism](clish]) for the information about using new features of COMMAND tag.

#### [lock]
 `[A boolean flag. It can enable (true) or disable (false) the [locking_mechanism locking mechanism](lock]`) for the current command.

Default is true.

#### [ref]
 `[The 'ref' field is used to create a [command_alias command alias](ref]`). If the 'ref' field is used within COMMAND definition that command is not standalone but it's an [alias](command_alias). The 'ref' contain the name of target original command to make alias of. In the case if the target command belongs to the another view than the view of alias then the target command's view must be specified after the target command name. The delimeter beetween the command name and view name is "@" symbol. See the [command alias](command_alias) page for the details and examples.

#### [interrupt]
 `[The 'interrupt' field specifies if the [ACTION](interrupt]`) script is interruptable or non-interruptable by the user. If the interrupt="true" than the script is interruptable else the script is non-interruptable. For non-interruptable scripts the SIGINT and SIGQUIT is temporarily blocked. See the [atomic actions](atomic_action) for the details. The 'interrupt' field is available since SVN revision 347 or Klish-1.4.0.
 



### CONFIG

The CONFIG tag was implemented to support interaction beetween Klish engine and some external (or internal) mechanism to store a commands sequence i.e. CISCO-like configuration.

#### [operation]
Defines the action on current configuration (running-config):

- set - write currently entered command line to the running-config. If the command is already in the running-config it will be no changes. The "pattern" field define the uniqueness of command. If the running-config already contain entries starting with the "pattern" than these entries will be removed.
- unset - remove entries from the running-config due to specified "pattern".
- dump - write the running-config to the specified "file". If the "file" is not specified than running-config will be written directly to the communication channel (the socket in the case of "[konfd](operation]`)" configuration backend). So the config callback function must care about data receiving. The standard callback can receive and show the data from "[daemon.

The default is "set".
 
#### [priority]

The "priority" field define the sort order within running-config. Note the order of commands is important. For example to setup routing table the interfaces must be already configured.

The "priority" is a two-byte hex number (for example "0x2345"). The high byte defines the configuration group of command. The low byte defines the priority within the group. The configuration groups is separated from each other with "!" symbol. The commands within group can be separated or not separated with "!". The separation behaviour within group is defined by "splitter" field. For example the CISCO-like running-config will separate the definitions of interfaces with "!" but will not separate "ip route ..." and "ip default-gateway ..." commands.

The default is "0x7f00". It's a medium value of the high-byte.

#### [pattern]
The field specify the pattern to remove entries from running-config while "unset" operation and the identifier of unique command while "set" operation.

The default is the name of the current command (`${__cmd}`).

#### [file]

This field defines the filename to dump running-config to.

#### [splitter]
A boolean flag. The allowed values is true or false. If the "splitter" is "true" than the current command will be separated with the "!" symbol within its configuration group. See the "priority" description for details about configuration groups.

Default is true.

#### Notes

The CISCO-like config supports nested commands. It uses indention as a syntax for the nesting. To specify nesting depth of command the "depth" option of [VIEW](splitter]`) tag is used. All the commands of view have the same depth.




### HOTKEY

The HOTKEY tag allows to implement programmable hotkeys. The global view (XML configuration without explicit view definition) and [can contain HOTKEY tags. See [hotkeys](VIEW]s) page for additional information.

The HOTKEY tag was implemented since Klish-1.5.7 and Klish-1.6.2.

#### key
The symbolic key description. The Klish supports control keys with "Ctrl" ("`^`" symbol) only. Some combination are internally reserved (like a Ctrl`^`C and some other keys). To define a key use "`^[key_simbol](key]`)`". For example:

```
<HOTKEY key="^Z" .../>
<HOTKEY key="^S" .../>
```

The first line is for `Ctrl^Z` and the second is for `Ctrl^S` combinations accordingly.

#### cmd
The Klish [COMMAND](cmd]`) with arguments to execute on specified hotkey combination. This command must be defined in XML config. The command string can contain dynamically expanded [VAR]s.

```
    <HOTKEY key="^Z" cmd="exit"/>
    <HOTKEY key="^@" cmd="show running-config"/>
    <HOTKEY key="^S" cmd="echo ${HOSTNAME}"/>
    <VAR name="HOSTNAME" ... />
```



### NAMESPACE

The NAMESPACE tag allows to import the command set from the specified view into another view. See the [logically nested views](nested_views) for details on using this tag.

#### ref

Reference to the view to import commands from.

#### [prefix]
The prefix for imported commands.

#### [inherit]

A boolean flag whether to inherit nested namespace commands recursively. Can be true or false. Default is true.

#### [help]
A boolean flag whether to use imported commands while help. Can be true or false. Default is false.

#### [completion]

A boolean flag whether to use imported commands while command completion. Can be true or false. Default is true.

#### [context_help]

A boolean flag whether to use imported commands while context help. Can be true or false. Default is false.




### PARAM

The PARAM tag defines command parameters. This document describes Klish native options only. See the [documentation for  the other PARAM options. See the [optional_arguments optional arguments](clish]), [subcommands](subcommands) and [switch subcommands](switch_subcommand) for the information about using new features of PARAM tag.

#### [optional]
A boolean flag. Specify whether parameter is optional. The allowed values is true or false. 

Default is false.

#### [order]

A boolean flag. Can be used with optional (optional="true") parameters only. If current parameter is specified in command line then previously declared (in XML file) other optional parameters can't be entered later. So this option set the order of available optional parameters. See the [Optional arguments](optional_arguments) page for example.

The allowed values is true or false. Default is false.

The feature is available since version 1.5.2 or SVN revision #522.

#### [mode]
Define parameter behaviour. It can be:

- common - the standard mode for ordinary parameter. Nothing special.
- subcommand - the parameter is [subcommands subcommand](mode]`). The subcommand is identified by its "name" (or "value" field if specified) and can be used as optional flag or for the branching. If the subcommand was used then the value of parameter is its name ("value" field if specified). The value of parameter is undefined if parameter is optional and was not used. See the [subcommand](subcommands) and "value" field documentation for details.
- switch - the parameter is [switch subcommand](switch_subcommand). The switch subcommand's sub-parameters is alternative and allow branching implementation. The parameter itself get the name of choosen sub-parameter as a value. See the [switch subcommand](switch_subcommand) documentation for details.

Default is "common".

#### [value]
The [subcommands subcommand](value]`) specific option. This field is used to separate the name of internal variable and the displayable name (that user will enter). The "name" field is a name of the internal variable. The "value" is a displayable subcommand name. It allows to duplicate displayable subcommand names.

The "value" field forces the mode of PARAM to "subcommand".

The feature is available since version 1.2.0.

#### [hidden]
The 'hidden' field specify the visibility of the parameter while [internal_variables `${__line}`](hidden]`) and [`${__params}`](internal_variables) automatic variables expanding. The expanding of variable with the PARAM name is performed by the usual way. The allowed values is "true" or "false".

Default is "false".

For example this feature can be used while the [ordered sequences](sequence) implementation. The hidden parameter can specify the line number in [ordered sequence](sequence). So it must be passed to the [daemon via [sequence](konfd]) field of CONFIG tag but the `${__line}` (that will be set to the user config) doesn't need to contain line number.

#### [test]
The parameter can be dynamically enabled or disabled depending on the condition. The condition have the syntax same as standard /bin/test utility. So the parameter visibility can depend on the previous parameters values and [internal_variables internal variables](test]`). See the [conditional parameters](conditional_param) for details.

By default the parameter is enabled.




### STARTUP

The STARTUP tag defines the starting view, viewid and the other startup settings. This document describes Klish native options only. See the [documentation for the other STARTUP options.

#### [default_shebang]

Defines the scripting language (the binary file) to use for the [script execution by default.

Default is the "/bin/sh". The [ACTION](ACTION]) tag with 'shebang' field can locally redefine the shebang for its execution.

#### [timeout]
Without any user activity for the specified timeout the Klish can autologout (close current input stream and exit). It can be used to automatically close privileged sessions when the administrator have forgot to close session manually.

#### [lock]

The same as "lock" field of [tag.

#### [interrupt]

The same as "interrupt" field of [COMMAND] tag.

### VIEW

The VIEW tag defines a view. The view aggregates the commands.

#### [depth]
A depth of nested view. It is used together with the [CONFIG](depth]`) tag. If the command must be written to the config the view's depth specifies the command indention within [CISCO-like config](cisco_config). All the commands within current VIEW have the same depth.

The default is "0".

#### [restore]
The commands contained by the view can be executed from the nested views or parallel views using the [NAMESPACE](restore]`). While the command execution the depth (and a context) or the view of command can be restored. The value of 'restore' field can be:

- none - Don't change the current view.
- view - The current view will be set to the command's native view.
- depth - The Klish engine will find out the depth of command's native view. Then it will search for this depth in the current nested views stack. The current view will be set to the saved view from the stack with the depth equal to command's depth. Additionally the context (the viewid) will be restored from the stack.

Default is "none". See the [nested views](nested_views) wiki page for the additional information and example.





## Utilities

# The initial view redefinition.

<wiki:toc max_depth="2"/>

# Introduction

User can define CLISH_VIEW environment variable to set initial view. This value will be used instead of the initial view from STARTUP tag.

The feature is available starting with Klish-1.1.0.

# CLISH_VIEWID

The CLISH_VIEWID environment variable can redefine the viewid field from STARTUP tag.

The feature is available starting with Klish-1.1.0.# The daemon to store running-config.

<wiki:toc max_depth="2"/>

# Introduction

The konfd is a daemon to store current running-config. You can consider running-config as a current system settings or as a list of commands that have been executed for now by the user or automatically (by the script).

The name "konfd" is used since Klish-1.1.0. The earlier versions use name "confd" for the configuration daemon.

# The running-config

Generally the running-config consists of the the arbitrary text lines. Each entry may contain another nested text lines. Note the **konfd knows nothing about Klish commands and Klish syntax**. The commands is just a **text strings** for the konfd. It's important to realize. The example of running-config:

<code>
hostname Router
!
interface ethernet 0
 ip address 192.168.1.1/24
 enable
!
interface ethernet 1
 ip address 10.0.0.1
</code>

The "hostname Router" entry has no nested entries. The "interface ethernet 0" and "interface ethernet 1" contain nested entries.

## Comments

The "!" symbol is a comment for the Klish. Each line starting with the "!" consider as a comment so this line will be ignored by the Klish. The konfd daemon can output a current state of running-config. The previous text is an example of such output. Generally the comments are not really stored by the konfd. The konfd automatically inserts "!" beetween first level running-config entries for more human readable view.

## Path

The running-config structure can be considered as a filesystem-like structure. The "interface ethernet 0" entry can be considered as a directory with the nested "files". So each entry has a "path". The first level entries have an empty path but the nested entries like a "enable" entry has a non-empty path:
<code>
"interface ethernet 0"
</code>
The running-config can contain multi level nesting:
<code>
interface ethernet 0
 ip address 192.168.1.1/24
 ip options
  mtu 1500
 enable
</code>
In this case the "mtu 1500" entry has a following path:
<code>
"interface ethernet 0" "ip options"
</code>
The "path" concept is important for running-config entries manipulations.

## Priority

Each entry has a priority. The priority is a 16-bit unsigned integer. The entries with minimal priority resides on the begining of the running-config. The default priority is "0". The entries with the same priority will be ordered alphabetically.

The priority can be specified in hex format. The Klish use "0x7f00" default value for the priority if priority is not specified explicitly within [CONFIG] tag. The "0x7f00" is a middle of the possible priority range.

The high and low bytes within priority value have a little different meanings. The first level entries with different high bytes will be splitted by "!" (comment sign) always. By default the entries with equal high byte and arbitrary low byte will be splitted by "!" too. But if the entry has a "non-split" flag (see konfd protocol description to find out how to set this flag) the "!" will not be inserted before current entry if previous entry has the same high byte. So the entries can be grouped and don't be splitted by the "!".

The high and low bytes within priority value have no special meanings for nested entries.

## Sequences

The konfd supports ordered lists a.k.a. "sequences". The entry can be or not to be a part of the sequence. It can be specified by a special options while entry creation. All entries in sequence must have the same priority value. The priority can be considered as an identifier of the sequence. The running-config can contain many sequences at the same time. The sequences will be identified by the priority value. 

The new entry can be inserted into sequence with specified sequence number. The entry can be removed from the sequence by its sequence number.

The konfd can output entries without or with sequence numbers prepending the entry value.

See the konfd communication protocol description for detail about sequence using.

# Communicate to konfd daemon

The konfd daemon is accessible via UNIX socket interface. The socket path can be specified via command line. So it's possible to have a several konfd executed simultaneously. The default socket path is /tmp/konfd.socket.

The konfd uses text based protocol for communication with another processes. The syntax of protocol is like a command line with options. It will be documented later in this document.

# Options

## `-v, --version`

Print the version of clish utility.

## `-h, --help`

Print help.

## `-d, --debug`

Enable debug mode. Don't daemonize konfd.

## `-s <path>, --socket=<path>`

Specify the UNIX socket filesystem path to listen on.

## `-p <path>, --pid=<path>`

File to save daemon's PID to.

## `-r <path>, --chroot=<path>`

Path to chroot to. Used for security reasons.

## `-u <user>, --user=<user>`

Execute daemon as specified user.

## `-g <group>, --group=<group>`

Execute process as specified group.

# The konfd protocol

The syntax of protocol is like a command line with options. The actions and options are specified by the arguments prepend with "-" or "--" (for long options) and after all arguments the "path" is specified. Each element of path must be quoted. Firstly the action must be specified:

## Add entry: `-s, --set`

To add new entry to the running-config the `"-s"` or `"--set"` argument is used. The following arguments are mandatory for this action:
- `-l <string>, --line=<string>`. This argument defines the text line to add to the running-config.
- `-r <regexp>, --pattern=<regexp>`. This argument contain extended regular expression and defines unique part of the entry i.e. the existing entries matching this pattern will be removed before new entry creation.
- The path to store new entry. This argument can be empty if entry must be added to the first level.

Examples:
<code>
-s -l "interface ethernet 0" -r "^interface ethernet 0$"
</code>
This example will add new entry "interface ethernet 0" to the first level of the running-config. If the entry "interface ethernet 0" already exists it will be overwritten by the same entry. The definitions of another interfaces (with another interface numbers) will not be removed because the regular expression contain the number "0" at the end of the pattern.

<code>
-s -l "ip address 192.168.0.1/24" -r "^ip address " "interface ethernet 0"
</code>
This example will add new nested entry "ip address 192.168.0.1/24" to the "interface ethernet 0" path. If the IP-address was defined before this action the old entry matching the "^ip address " pattern will be replaced by the new address. Suppose the entry "interface ethernet 0" already exists.

<code>
-s -l "mtu 1500" -r "^mtu " "interface ethernet 0" "ip options"
</code>
This code will add "mtu 1500" nested entry to the path "interface ethernet 0" "ip options". Suppose the path entries already exist. The running-config output after this operation is:
<code>
interface ethernet 0
 ip options
  mtu 1500
</code># The sigexec utility.

<wiki:toc max_depth="2"/>

# Synopsis

*`sigexec [<command to execute>`*

# Description

The sigexec utility unblocks (by sigprocmask()) all signals and executes specified command. It's usefull within [atomic_action non-interruptable](options]) [ACTION] scripts for daemon starting. The daemon will have clean signal mask.

# Options

## `-v, --version`

Print the version of utility.

## `-h, --help`

Print help.

### clish{#utility_clish}
Command line interface shell.

<wiki:toc max_depth="2"/>

# Synopsis

*`clish [[script_filename](options]) [...`*

# Description

This clish is command line interface shell. The available shell commands and its actions are defined by XML configuration files. The clish utility can get input commands from terminal in interactive mode, from files specified in command line (multiply "script_filename" arguments) or standard input.

# Options

## `-v, --version`

Print the version of clish utility.

## `-h, --help`

Print help.

## `-s <path>, --socket=<path>`

The clish utility can work together with the [konfd](script_filename]) daemon. This daemon can store commands entered in clish. It's usefull to implement config file like CISCO's running-config that stores current system configuration i.e. sequence of commands to achieve current system state. The command sequence can be saved to file (CISCO's startup-config) and executed in batch mode later by clish utility.

The [daemon listens for connections on UNIX domain socket. You can specify the filesystem path to this UNIX domain socket to connect to.

## `-l, --lockless`

Don't use locking mechanism.

## `-e, --stop-on-error`

Stop programm execution on error.

## `-b, --background`

Start shell using non-interactive mode.

## `-q, --quiet`

Disable echo while executing commands from the file stream.

## `-d, --dry-run`

Don't actually execute ACTION scripts.

## `-x <path>, --xml-path=<path>`

Path to XML scheme files.

## `-w <view_name>, --view=<view_name>`

Set the startup view.

## `-i <vars>, --viewid=<vars>`

Set the startup viewid.

## `-u, --utf8`

Force UTF-8 encoding.

## `-8, --8bit`

Force 8-bit encoding.

## `-o, --log`

Enable command logging to syslog's local0.


# Environment

# Files

# Return codes

The clish utility can return the following codes:

- *0* - OK
- *1* - Unknown internal error.
- *2* - IO error. Can't find stdin for example.
- *3* - Error while the [ACTION](konfd]) script execution.
- **4** - Syntax error.
- **255** - The system error like wrong command line option for clish utility.

# Notes

The return codes are available since Klish-1.5.2 or SVN's revision #516.# One-sentence summary of this page.

<wiki:toc max_depth="2"/>

# Introduction

Te page is under construction.


# Details
# The buildroot environment

<wiki:toc max_depth="2"/>

# Introduction

Buildroot is a set of Makefiles and patches that makes it easy to generate a complete embedded Linux system. See the [http://www.buildroot.net/] for the details. The Klish can be used as the "package" with the buildroot environment to get into the buildroot's target embedded system.

# Details

The Klish source tree contain the contrib/buildroot directory with the files to embed the Klish package into the buildroot environment. The contrib/buildroot/package/Klish must be copied to the buildroot's source tree package/Klish directory. Then the package/Config.in file must be changed. Add the following line

    source "package/Klish/Config.in"

to the section started with 'menu "Shell and utilities"' (or to the another
section if you think it will be better and you know what you do). After that you
can configure buildroot and enable the building the Klish package. You can find
the menu for the Klish within "Package Selection for the target"->"Shell and utilities" in the configurator. The Klish's config allow to use the predefined version of Klish or automatically download the last revision from the SVN repository.

These files were tested with buildroot-2010.11.
The current predefined stable version of Klish package that used in the
buildroot's Klish.mk file is 1.3.1.# Supported XML backends

<wiki:toc max_depth="2"/>

# XML backends

The Klish engine uses external (since version Klish-1.6.0) XML backends to parse XML config files. Now Klish supports three XML backends:

- libxml2
- expat
- libroxml

There are XML tests in xml-examples/test directory in Klish sourcecode tree. These tests contain some typical cases that can lead to XML backend errors.

You can specify preferred XML backend by project configure script argument:

- --with-libxml2 for using libxml2
- --with-libexpat for using expat
- --with-libroxml for using libroxml

## libxml2

The libxml2 project homepage is [The tested versions are:
### 2.7.8

The Klish-1.6.0 tests are passed on:
- Linux (ArchLinux)
- FreeBSD 8.2

## expat

The expat project homepage is [http://expat.sourceforge.net/](http://www.xmlsoft.org/].).
The tested versions are:
### 2.0.1

The Klish-1.6.0 tests are passed on:
- Solaris 11
- FreeBSD 8.2
### 2.1.0

The Klish-1.6.0 tests are passed on:
- Linux (ArchLinux)

## libroxml

The expat project homepage is [Don't use versions earlier than libroxml-2.2.0.
The tested versions are:

### 2.1.1

 The Klish-1.6.0 test are passed. 
- FreeBSD 8.2
 Probably the recent version of libroxml can be build on this systems.

### 2.2.0

 The Klish-1.6.0 tests are NOT passed. The "test5" is failed (broken quotes within comment). Now libroxml git repository contain the patch to solve this problem. It was tested on:
- Linux (ArchLinux)

# Legacy backend

The Klish earlier than Klish-1.6.0 version uses internal implementation of [http://sourceforge.net/projects/tinyxml/ tinyXML]([https://code.google.com/p/libroxml/].) engine. It was frozen snapshot and it was rather good but tinyXML is written in C++. So Klish can't be build without C++. It's not good for embedded devices. The Klish-1.6.0 and later versions can be build without C++.

# FreeBSD note

The FreeBSD use "ports" system for third party open source projects. All ports are installed to /usr/local. So the Klish configuration with installed XML backends is something like this:
    # CPPFLAGS="-I/usr/local/include" LDFLAGS="-L/usr/local/lib" ./configure --with-libxml2

# Solaris note

The Solaris 11 has no pkg-config package. The configure script use pkg-config to search for libxml2 headers and libs. So to build Klish with libxml2 backend you need to configure libxml2 path manually:
    # ./configure --with-libxml2=/usr
    
    
# HOWTO

<wiki:toc max_depth="2"/>

# Introduction

The HOWTO.

# The XML files validation

The XML files can be validated by the xmllint utility.
            $xmllint --noout --schema <schema_filename>.xsd <xml_filename>.xml
Or you can validate all XML file in the specified dir:
            $xmllint --noout --schema <schema_filename>.xsd <xml_dir>/*.xml

# Static build

To build Klish statically use:
<code>
$ ./configure --disable-shared
$ make LDFLAGS+="-all-static"
</code>

The LDFLAGS is global so shared libraries can't be build and building of shared libraries must be disabled.

# Leak of dlopen()

If target system doesn't support dlopen() then configure script will configure building process to don't use dlopen() (and other dl functions) but link to plugin's shared objects.

If you need to link statically with plugins use:
<code>
$  ac_cv_header_dlfcn_h=no ./configure --prefix=/usr --with-lua --disable-shared
$ make LDFLAGS+="-all-static"
</code># The Klish XML examples.

<wiki:toc max_depth="2"/>

# Introduction

The Klish source tree contain the Klish specific XML examples that show the basic CISCO-like (not exactly copy) interface to configure network interfaces and routing on Linux system. You can find it in xml-examples/Klish dir in the Klish source tree.

The original clish examples is also available and workable. You can find it in xml-examples/clish dir.

# The KLISH specific examples

The Klish has some new features that is not supported in clish. So the Klish specific examples show some of these new features. The dir xml-examples/Klish is more complex than clish examples dir. That is needed to show CISCO-like 'enable' command that allow to get privileged mode and execute administration commands.

## The 'enable' command implementation

The 'enable' command is implemented by using 'su' linux command and executing new clish instance. It's the right way to get privilegies because the clish/Klish is not enough tool to distribute system permissions. When the 'su -c clish' is used the operation system is responsible to permit or deny some system operations to the current user. For example unprivileged user cannot change the network interfaces settings and routing table. So if even the unprivileged user will be able to enter network specific commands in Klish the system will deny his commands.

If the some kind of role model is needed the 'su' command can be used to became non-root user with additional permissions. The additional permissions can be set using 'sudo' for example.

## Directory structure

The directory structure resulting from the realization of 'enable' command. The privileged and unprivileged users must have the different set of XML files. The example suppose that Klish was configured with './configure --prefix=/usr' so the installed clish binary will be located in /usr/bin dir. The 'etc' dir of the example must be copied to the / dir to the target system.

The etc/ dir contain clish/ clish-enable/ and clish-xml/ dirs. The clish-xml/ dir contain the all (privileged and unprivileged) XMLs. The clish/ dir contain symbolic links to the ../clish-xml/`<name>`.xml files that is needed in non-privileged mode. The clish-enable/ dir contain symbolic links to the ../clish-xml/`<name>`.xml files that is needed in privileged mode.

The etc/ also contain init.d/Klish-init script that will init Klish subsystem on the boot time. It starts the konfd daemon that will store all the configuration (all the configuration commands user will enter). Then the saved configuration file /etc/startup-config is executed via Klish to restore previous (pre-reboot) system configuration. The init.d/Klish-init script can be included in some init script like rc.local so it will be executed automatically on system startup.

## The testing purposes only

If you don't want to install all Klish infrastructure to your system and don't want to use 'enable' command you can use the following commands to see the ability of unprivileged and privileged examples (suppose the current dir is a Klish source tree):

    ~/Klish$ CLISH_PATH=xml-examples/Klish/etc/clish bin/clish
    ~/Klish$ CLISH_PATH=xml-examples/Klish/etc/clish-enable bin/clish

Note privileged commands can be entered by the common user but the real system commands execution will be denied.

# The CLISH original examples

To test Klish over the clish original examples the project must be configured and built. Unarchive the source code tarball and 'cd' to the klish-`<version>` tree. Then execute the following commands:

    $ ./configure
    $ make
    $ CLISH_PATH=xml-examples/clish bin/clish