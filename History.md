# Looking Glass 2.7 #
  * Added data query functionality to view options: Now it is possible to enter strings containing lists of station names (i.e.: **F3,F4,E5**) to select stations, instead of having to click on them in the 3d view.
  * Added a new and customizable geographic data viewer. Supports unlimited image overlays, 3d meshes and point sets. Each 3d model can be cunfigured and clipped separately.
  * Added a line tool. It allows to graphically specify intervals in the 3d view and use them for clipping geographic data.
  * Major improvements to plot views:
    * Added support to specify precise data ranges
    * Customizable major and minor tick intervals
    * Customizable reference lines for the X and Y axes
    * Now plots display lines in the order in which they are selected (either through queries or by direct selection in the 3d view)
## Issues ##
  * Plot line colors may turn out incorrect when enabling and disabling reference lines. Re-selecting the data usually fixes this
  * Slice viewer and Navigation View tools have been disabled in this version of Looking Glass.


---


# Looking Glass 2.6.1 #
First version on google code. No significant code changes, just fixed some minor dataset issues

---


# Looking Glass 2.6 #
## Changes ##
  * Added plot label options.
  * Fixed plot labels and plot curve generation in general is working much better.
  * Mission / dive selector is more intuitive.
  * Table view allows selection of columns.
  * Table view shows only selected data if something is selected. Filtered data otherwise.

---


# Looking Glass 2.5 #
## Changes ##
  * Added plot preference support
## Known Issues ##
  * The sonde-based bathymetry model is not being generated in this version.
  * Sonde drop picking is still acting weird on some drops (picking the wrong drop)
  * Plot legend and curve coloring is not working for some drops. Clicking two times on a drop appears to solve this issue sometmes

---


# Looking Glass 2.4 #
## Changes ##
  * Dataset supports a new additional Tag field (Tag3)
  * Added support multiple drop picking (ctrl-click)
  * Optimized initial data loading
  * Mission range tool now updates plots in real time
## Known Issues ##
  * DataField manager add field button is not working. Right now derived fields have to be added by hand in the application configuration file.
  * The sonde-based bathymetry model is not being generated in this version.
  * Sonde drop picking is still acting weird on some drops (picking the wrong drop)
  * Plot legend and curve coloring is not working for some drops. Clicking two times on a drop appears to solve this issue sometimes

---


# Looking Glass 2.3 #
## Fixes ##
  * Fixed plot lines for multiple sonde drops on the same station.
## Changes ##
  * Dataset supports an additional Tag field (Tag2)
  * Added support for data grouping by tag (used mainly for plot generation as for now)
  * Removed expression filter from table view
  * Removed the depth cutoff filter and other stuff not used anymore
  * Added legend to plots
  * (ENDURANCE HACK) Plot shows different colors for data of different years.
## Known Issues ##
  * DataField manager add field button is not working. Right now derived fields have to be added by hand in the application configuration file.
  * The sonde-based bathymetry model is not being generated in this version.

---


# Looking Glass 2.2 #
## Changes ##
  * Application now supports automatic updates.
  * Plot window supports zooming
  * Plot window supports exporting the plot as a PNG image
  * Selection now selects filtered data instead of the entire dataset.
  * (ENDURANCE HACK) Mission selector allows selection of 2008 or 2009 mission data.
  * (ENDURANCE HACK) Added a tool to specify sonde data depth cutoff.
## Known Issues ##
  * Evaluation of filter expressions in table view and derived field calculations are pretty slow, and need to be optimized.
  * DataField manager add field button is now working. Right now derived fields have to be added by hand in the application configuration file.
  * Sonde drop picking is working incorrectly sometimes.
  * When selection a station where multiple drops have been performed, the plot line draws are incorrect.
  * The sonde-based bathymetry model is not being generated in this version.

---


# Looking Glass 2.1 #
## Fixes ##
  * DLL problem solved for win32 release.
## Changes ##
  * Added calculation of sound speed models (Chen/Millero and DelGrosso)
  * Added a new configuration profile for the application (Bonney2009Preproc). This profile loads a preprocessed dataset instead of recalculating derived fields (i.e. sound speed models) at startup, making the application quicker to start. If new fields are added or the calculations are modified, it is still possible to load the original dataset using the Bonney2009 profile.
  * The table view tool supports custom filter expressions.
  * The table view tool supports saving the current dataset as a csv file.
## Known Issues ##
  * Evaluation of filter expressions in table view and derived field calculations are pretty slow, and need to be optimized.
  * DataField manager add field button is now working. Right now derived fields have to be added by hand in the application configuration file.
  * Sonde drop picking is working incorrectly sometimes.
  * When selection a station where multiple drops have been performed, the plot line draws are incorrect.
  * The sonde-based bathymetry model is not being generated in this version.

---


# Looking Glass 2.0 #
## Changes ##
  * First 2.x version released.
## Known Issues ##
  * Sonde drop picking is working incorrectly sometimes.
  * When selection a station where multiple drops have been performed, the plot line draws are incorrect.
  * The sonde-based bathymetry model is not being generated in this version.