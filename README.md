<h3> 
Classification of Three-dimensional Geometric Objects using C++ and Multithreading
</h3>
<p>
This program is written in C++11 and used Eclipse CDE 2025-06 IDE with GCC compiler. 
The program takes advantage of the C++ Multi-Thread concurrency library. It creates 154 threads: 
main thread, 3 axis threads (i,j,k), and 50 plane threads for each axis.  The square-error results from
each plane of each axis are pushed to synchronized FIFO queues that make use of mutexes in 
the critical sections to avoid race conditions. As each axis plane finishes its processing to find
the minimum square error, it attempts to acquire a unique lock with the mutex on the shared FIFO queue.
After it pushes its square-error to the tail of the queue, it unlocks the unique lock.  Each axis thread
joins with each plane thread it created and the main thread joins with each axis thread it created.  There
is no resource leakage caused by detached threads.
</p>

<p>
Geometric Classification classifies the internal structure of 3D geometric objects
such as ellipsoids, parabloids, cubes, boxes, planes, lemniscates,
cardiods, four-leaf rose, torus, or cones.  It slices the geometric
objects along axial planes in the Cartesian coordinate system.
The objects can be solids as well as surfaces. The program provides a 
display geometric option to display a particular class, axis, and range
of planes. The planes are displayed at two-second intervals. 
It will classify the geometric object and display the results. It does this
by comparing the noisy test samples that are displaced randomly in space with
references of the geometric objects that are noise free and centered.  The metrics
are mass sums of the rows and columns of plane in each axes in the Cartesian
coordinate system.  The least square error determines how the sample is classified.
The difference between the reference class mass sums and the test sample is the error.
</p>

<p>
Since the classification involves 3-dimensional searches, it takes a long time to 
finish execution.  For classification of 100 samples of the 20 geometric objects,
it took about six hours.  The geometric objects are represented by densities which
can be any concentration quantity.  By summing up the row or column densities you
can obtain a quantity proportional to mass.	Thus the row sums and columns sums 
distinguish different geometric objects and can be used to classify the object.
The multi-threaded implementation is about 2-3 times faster than the sequential
single-thread implementation.
</p>

<p>
  To display a particular geometric object, enter the class number, axis, the start
  plane, and the stop plane.  The planes will be flashed at two-second intervals.
  The density values are typed in color.  Here is a list of the geometric object
  and their class number:
    <ol>
	  <li>ellipsoidsurface</li>
	  <li>ellipsoidsolid</li>
	  <li>plane</li>
	  <li>paraboloid</li>
	  <li>paraboloidsolid</li>
	  <li>cube</li>
	  <li>cone</li>
	  <li>conesolid</li>
	  <li>box</li>
	  <li>hyperbolicparaboloid</li>
	  <li>cylindersurface</li>
	  <li>cylindersolid</li>
	  <li>potentialwell</li>
	  <li>cardioidrevolution</li>
	  <li>cardioidrevolutionsolid</li>
	  <li>lemniscaterevolution</li>
	  <li>lemniscaterevolutionsolid</li>
	  <li>rose4leafrevolution</li>
	  <li>rose4leafrevolutionsolid</li>
	  <li>torussolid</li>
	</ol>
  </p>


<h4>Classification of 100 samples, level 0 noise, no random displacement, 20 geometric objects.</h4>

<pre>
C:\Users\Thoma\eclipse-workspace\MultiThreadGeometricClassification\Debug>MultiThreadGeometricClassification.exe
threads available = 12
Choose Classify Geometric = 1 or Display Geometric = 2:  1
---------- Geometric Classification Running ----------
Start local time and date: Sun Jun 14 09:39:44 2026

Enter the number of samples, noise level, and shift, separated by space
50<=[samples]<= 500, 0<=[noise level]<= 9, [0=false|1=true]
For example, '50 3 1'
100 0 0
You entered: 100 0 0
Geometric geo constructed, start classifyGeometric
classifyGeometric, start loop over the samples

**************************************************
**************************************************

leaving classifyGeometric
tabulateTestResults
displayTestResults

|======================================================|
|Class  |Geometric                 |Count  |Correct (%)|
|=======|==========================|=======|===========|
|0      |ellipsoidsurface          |4      |100        |
|1      |ellipsoidsolid            |3      |100        |
|2      |plane                     |7      |100        |
|3      |paraboloid                |5      |100        |
|4      |paraboloidsolid           |7      |100        |
|5      |cube                      |4      |100        |
|6      |cone                      |3      |100        |
|7      |conesolid                 |4      |100        |
|8      |box                       |9      |100        |
|9      |hyperbolicparaboloid      |9      |100        |
|10     |cylindersurface           |5      |100        |
|11     |cylindersolid             |7      |100        |
|12     |potentialwell             |5      |100        |
|13     |cardioidrevolution        |6      |100        |
|14     |cardioidrevolutionsolid   |1      |100        |
|15     |lemniscaterevolution      |4      |100        |
|16     |lemniscaterevolutionsolid |2      |100        |
|17     |rose4leafrevolution       |10     |100        |
|18     |rose4leafrevolutionsolid  |3      |100        |
|19     |torussolid                |2      |100        |
|------------------------------------------------------|
|Totals |                          |100    |100        |
|======================================================|

Finish local time and date: Sun Jun 14 16:03:40 2026  
</pre>

<h4>Classification of 100 samples, level 3 noise, random displacement, 20 geometric objects.</h4>

<pre>
C:\Users\Thoma\eclipse-workspace\MultiThreadGeometricClassification\Debug>MultiThreadGeometricClassification.exe
threads available = 12
Choose Classify Geometric = 1 or Display Geometric = 2:  1
---------- Geometric Classification Running ----------
Start local time and date: Mon Jun 15 12:28:42 2026

Enter the number of samples, noise level, and shift, separated by space
50<=[samples]<= 500, 0<=[noise level]<= 9, [0=false|1=true]
For example, '50 3 1'
100 3 1
You entered: 100 3 1
Geometric geo constructed, start classifyGeometric
classifyGeometric, start loop over the samples

**************************************************
**************************************************

leaving classifyGeometric
tabulateTestResults
displayTestResults

|======================================================|
|Class  |Geometric                 |Count  |Correct (%)|
|=======|==========================|=======|===========|
|0      |ellipsoidsurface          |7      |100        |
|1      |ellipsoidsolid            |2      |100        |
|2      |plane                     |0      |0          |
|3      |paraboloid                |6      |100        |
|4      |paraboloidsolid           |5      |100        |
|5      |cube                      |5      |100        |
|6      |cone                      |5      |100        |
|7      |conesolid                 |3      |100        |
|8      |box                       |3      |100        |
|9      |hyperbolicparaboloid      |5      |100        |
|10     |cylindersurface           |4      |75         |
|11     |cylindersolid             |6      |100        |
|12     |potentialwell             |3      |100        |
|13     |cardioidrevolution        |8      |100        |
|14     |cardioidrevolutionsolid   |6      |100        |
|15     |lemniscaterevolution      |8      |100        |
|16     |lemniscaterevolutionsolid |4      |25         |
|17     |rose4leafrevolution       |7      |85         |
|18     |rose4leafrevolutionsolid  |5      |100        |
|19     |torussolid                |8      |87         |
|------------------------------------------------------|
|Totals |                          |100    |94         |
|======================================================|

Finish local time and date: Mon Jun 15 18:11:38 2026  
</pre>

<h4>Classification of 100 samples, level 5 noise, random displacement, 20 geometric objects.</h4>

<pre>
C:\Users\Thoma\eclipse-workspace\MultiThreadGeometricClassification\Debug>MultiThreadGeometricClassification.exe
threads available = 12
Choose Classify Geometric = 1 or Display Geometric = 2:  1
---------- Geometric Classification Running ----------
Start local time and date: Tue Jun 16 06:29:15 2026

Enter the number of samples, noise level, and shift, separated by space
50<=[samples]<= 500, 0<=[noise level]<= 9, [0=false|1=true]
For example, '50 3 1'
100 5 1
You entered: 100 5 1
Geometric geo constructed, start classifyGeometric
classifyGeometric, start loop over the samples

**************************************************
**************************************************

leaving classifyGeometric
tabulateTestResults
displayTestResults

|======================================================|
|Class  |Geometric                 |Count  |Correct (%)|
|=======|==========================|=======|===========|
|0      |ellipsoidsurface          |6      |100        |
|1      |ellipsoidsolid            |4      |100        |
|2      |plane                     |3      |100        |
|3      |paraboloid                |4      |100        |
|4      |paraboloidsolid           |2      |100        |
|5      |cube                      |3      |100        |
|6      |cone                      |6      |100        |
|7      |conesolid                 |2      |100        |
|8      |box                       |8      |100        |
|9      |hyperbolicparaboloid      |4      |0          |
|10     |cylindersurface           |9      |88         |
|11     |cylindersolid             |5      |80         |
|12     |potentialwell             |2      |100        |
|13     |cardioidrevolution        |5      |100        |
|14     |cardioidrevolutionsolid   |6      |100        |
|15     |lemniscaterevolution      |5      |80         |
|16     |lemniscaterevolutionsolid |6      |0          |
|17     |rose4leafrevolution       |5      |100        |
|18     |rose4leafrevolutionsolid  |8      |75         |
|19     |torussolid                |7      |100        |
|------------------------------------------------------|
|Totals |                          |100    |85         |
|======================================================|

Finish local time and date: Tue Jun 16 12:33:32 2026
</pre>

<h4>Display Geometric, paraboloid solid, axis 1, plane 25</h4>
<img width="988" height="997" alt="image" src="https://github.com/user-attachments/assets/41c6b943-d1c9-4e90-b0f8-322def2b4c53" />
<h4>Display Geometric, paraboloid solid, axis 0, plane 25</h4>
<img width="1058" height="970" alt="image" src="https://github.com/user-attachments/assets/7922db92-0d9a-4bcf-98ea-6f8c3c29ec90" />
<h4>Display Geometric, lemniscaterevolutionsolid, axis 0, plane 25</h4>
<img width="919" height="988" alt="image" src="https://github.com/user-attachments/assets/80a3dc5a-11df-4886-8fd8-12701e570caa" />
<h4>Display Geometric, torus solid, axis 0, plane 25</h4>
<img width="942" height="988" alt="image" src="https://github.com/user-attachments/assets/0b8fe81c-ab29-4885-8739-28516f4cf0c2" />
<h4>Display Geometric, torus solid, axis 1, plane 26</h4>
<img width="926" height="986" alt="image" src="https://github.com/user-attachments/assets/e4f25322-5675-4a01-8e38-d862d4ae5d5a" />
<h4>Display Geometric, torus solid, axis 1, plane 30</h4>
<img width="922" height="973" alt="image" src="https://github.com/user-attachments/assets/69b9faba-b995-4773-98e9-8f55500cedd1" />
