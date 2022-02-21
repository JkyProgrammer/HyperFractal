# Design

## High Level Design

As mentioned in the analysis section, I decided to allow both command line and GUI operation of my application. As such, my design can be effectively split into two main parts:
* the fractal generator
* the GUI

The fractal generator is the core of the application which does the actual processing and creation of the rendered image. It will be accessible from both command line and GUI modes, whereby either mode can create a rendering environment with the necessarry parameters (zoom, offset, equation, etc) which can be used for rendering universally.

In CLI mode, this can be done by parsing command line arguments and entering them into the rendering environment as parameters.

In GUI mode, this can be done instead by simply reading out values from the interface which the user can manipulate (e.g. the user presses a button which zooms in and this command is processed by changing a parameter in the rendering environment).

The GUI is effectively a separate module which provides the graphical interface to the user and simply modifies and calls into the rendering environment provided by the core module and displays the results on the screen.


The rendering environment will be encapsulated in a class which can be instantiated to create an independent, contained environment for generating fractals. It will contain class variables which can be assigned or retreived via getters and setters. It will also provide functionality to render fractals and to parse textual equations (see below), hiding all the code necessarry to do this from anything which utilises the environment (e.g. the GUI module).

The GUI environment will also be encapsulated in a class, which is host to the entire graphical system. It will be responsible for creating and managing the main application window, handling button presses and key events, and will keep track of the rendering environments (two will be used, one for the low resolution preview render, the other for the main, full-resolution render). The GUI class will contain various methods for handling presses for each button, navigation events like moving the viewport, as well as the main loop function which will manage drawing the interface each frame (this can also be broken down into separate methods). The only externally accessible method will be the GUI class's main function, which will perform setup such as intialising the class, creating the GUI and starting the graphics mainloop.

The `int main(int, char**)` function, which is called when the program starts, will contain code to handle either a console start or a GUI start. For a console start, it will parse the command line arguments, configure a rendering environment, run the render, save the result, and exit. For a GUI start, instead it will simply construct a GUI class and transfer control to the graphics main thread.

## Additional Detail

### Equation-parser
Other smaller subsystems will include the equation-parsing module, a set of functions to convert a mathematical expression (e.g. "(z^2)+c", which represents the Mandelbrot Set fractal) stored as a string into a form which can be evaluated rapidly at runtime. This module will define a number of data types to represent tokens which represent the parts of the equation. More information about the equation storage and parsing is below. In order to hide as much code as possible, only the outer-most of these functions will be accessible to the rest of the program, which can simply be called to perform the parsing and conversion and return an equation (see below).

### Database
Another element of the application is the ability to save details of configuration states (effectively saving the configuration of the rendering environment, along with a name), via a simple two-table database stored in CSV format. This will have its own standalone module, with a main class which can be used to manage, update, remove, or create records and handle saving and loading them automatically.

### Evaluator
The final module will be the evaluator, which contains the code to handle and evaluate equations parsed by the equation parser. It will hide the code to actually evaluate values for any given complex coordinate given, providing a function to do this which the main program can call for any pixel which needs evaluating. This will also be wrapped in a class to encapsulate as effectively as possible.

<bar></bar>

There will also likely be a utilities module which contains miscellaneous functions which multiple other modules might be using (such as a delay or wait procedure).

## Class Digram




## Flow Diagram

## Algorithms & Data Structures

### Parsing A Mathematical Expression
This is one of the most difficult elements of this project, simply due to the variability of possible cases. Equations will be stored using Reverse Polish Notation, also known as postfix notation. This can be easily stored in an array, and can be evaluated using a stack. Extracting the equation from a string is much harder however. I've decided to do this in a step-by-step process, as can be seen above. The string is first cleaned to remove whitespace, then checked to detect errors and invalid equations which would otherwise cause the parser to encounter problems. The parser can then step through the string and convert the sequence of characters into a sequence of tokens, which are much easier to process afterwards. This is done by tracking the type of the character currently under inspection (i.e. whether it is a digit, letter, '.', operation, etc) and using this to decide when a new token begins.
Handling brackets is the ideal situation to use a recursive function; this can be done by simply extracting the contents of the brackets and feeding it into the same tokenising function.

The next step of this is to turn the sequence of 'intermediate' tokens into a finalised array of tokens formatted as postfix notation (in the form `operand`,`operand`,`operation`). This is now mathematical processing, which means obeying mathematical rules, specifically the BIDMAS rule for order of operations. This is done simply by iterating over the sequence of tokens and pairing them up over an operation, and wrapping this in brackets (making postprocessing of the equation very easy). This is done in multiple passes, one per operation, so first indices, then division, and so on. However, another issue must be fixed as well. A common practice in mathematical notation is to use `ab` instead of writing `a * b`. This is known as implicit multiplication, and for the purposes of the equation parser it needs to be translated from the shortened form into the explicit form, by replacing `ab` with `(a*b)`. Order of operations still needs to be taken into account, so fixing implicit mulitplcation must be done between the 'division' and 'mulitplication' passes of the BIDMAS simplification phase.
However, once this is done, postprocessing of the equation is relatively simple. Because of the simplification phase, we can be sure that the sequence of 'intermediate' tokens will be formed only of `token`,`operation`,`token`, where each token may be a bracket (which contains another set of `token`,`operation`,`token`). In this way, tokens can be easily reordered into postfix notation; if they are simple tokens such as an operation or a number they can simply be copied out, otherwise they will be postprocessed recursively using the same function and the result will be inserted into the postfix token sequence.

This will generate a simple, linear, no-parsing-required sequence of instructions effectively which can be easily evaluated many times over by the fractal evaluator module.

### Evaluating Mathematical Expressions
After the majority of the work has been done by the parser, the fractal evaluator has minimal processing to do. This is deliberate, as the module must evaluate the equation many times (a maximum of `image size * image size * iteration limit` times, which on an average screen with average settings is very large). This is done by reading along the vector of tokens given by equation parser and treating each as an instruction. A stack is used to keep track of the computation state. Different tokens are treated differently:
* Number tokens will push a constant onto the stack with the value specified
* Letter tokens will be replaced with the relevant value (e.g. the letter 'z' will be replaced with the current value for z, see analysis for explanation of fractals) and pushed onto the stack
* Operation tokens will be executed over the top two items on the stack. These top items will be popped and the result of the operation will be pushed
At the end of the token sequence, there will be only one value left on the stack, which can be copied off and returned.

This is done many times over for each pixel, until the computed value is greater than the threshold, which is typically set a 2. Again, see analysis for explanation.

A major optimisation which can be made for this system is for built-in application presets. The speed of evaluating equations in this way is limited by the overhead of the stack operations. However, if the equation is already known (i.e. it is one of the application's presets) evaluating equations in this way is unecessarry, and so instead the equation presets (such as Mandelbrot and Julia sets) can be hard-coded and switched to at runtime, providing a major performance boost, since such hard-coded calculations avoid the overhead and can also be optimised heavily at compile-time by the compiler.

### Dividing Up Workload
Generating the entire fractal image requires a lot of processing, and would take a long time on a single thread. Nearly all modern computers have more than one logical processor, meaning they have the capacity to run multiple operations in parallel.
Multi-threading is key to achieving as realtime a result as possible. The command line arguments allow the user to specify the number of worker threads to use, whilst the GUI mode will automatically detect the optimal number of threads to use and configure accordinly.

From here, the workload of rendering the image must be divided up between the `n` threads. The mechanism to be used is very simple: a custom image class keeps track of not only computed pixel values, but also of whether or nor a pixel needs to be recalculated or not. This way, each worker thread simply requests a new pixel coordinate from the image manager, which then marks that pixel as 'in-progress'. The image class keeps track of which pixel should be assigned next to minimise wait times when threads request new pixels to compute. After each pixel is computed, it can be assigned back to the image class, which will flag the pixel as completed at the same time.

Some locking is required on the image manager class to prevent collisions, but this will be minimised in order to maintain performance without encountering collisions.

When the image is complete, the threads can be automatically halted as computation is complete.