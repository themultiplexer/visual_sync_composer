    1  git clone https://github.com/themultiplexer/visual_sync_composer
    2  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    3  ls
    4  git
    5  git clone https://github.com/themultiplexer/visual_sync_composer
    6  cd visual_sync_composer
    7  mkdir build
    8  cd build
    9  cmake ..
   10  brew install smake
   11  brew install cmake
   12  cmake ..
   13  brew install qt6
   14  cmake ..
   15  nano ../CMakeLists.txt
   16  cmake ..
   17  brew install kissfft
   18  brew install kiss-fft
   19  cd
   20  git clone https://github.com/mborgerding/kissfft.git
   21  cd kissfft
   22  \tmake KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   23  make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   24  brew install openmp
   25  brew install libomp
   26  make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   27    export LDFLAGS="-L/usr/local/opt/libomp/lib"\n  export CPPFLAGS="-I/usr/local/opt/libomp/include"
   28  make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   29  brew install gcc
   30  brew list gcc
   31  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   32  brew list libpng
   33  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   34  brew list libpng-dev
   35  brew install libpng
   36  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   37  brew install png
   38  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   39  brew install libpngbrew --prefix libpng
   40  brew --prefix libpng
   41  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/opt/libpng
   42  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   43  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 -L/opt/homebrew/lib all
   44  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all -L/opt/homebrew/lib
   45  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all LDFLAGS=-L/opt/homebrew/lib
   46  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all LDFLAGS="-L/opt/homebrew/lib"
   47  LDFLAGS="-L/opt/homebrew/lib" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   48  LDFLAGS="-L/opt/homebrew/lib" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   49  brew --prefix libpng
   50  LDFLAGS="-L/usr/local/opt/libpng" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   51  ls LDFLAGS="-L/opt/homebrew/lib"
   52  ls/usr/local/opt/libpng
   53  ls /usr/local/opt/libpng
   54  ls /usr/local/opt/libpng/lib
   55  LDFLAGS="-L/usr/local/opt/libpng/lib" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   56  LDFLAGS="-L/usr/local/opt/libpng/lib" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   57  make clean
   58  LDFLAGS="-L/usr/local/opt/libpng/lib" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   59  LDFLAGS="-L/usr/local/opt/libpng/lib -lpng" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   60  LDFLAGS="-L/usr/local/lib -lpng" CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   61  ls /usr/local/opt/libpng
   62  ls /usr/local/opt/libpng/lib
   63  nano Makefile
   64  ls
   65  nano CMakeLists.txt
   66  mkdir build
   67  cd build
   68  cmake ..
   69  brew install pkgconfig
   70  cmake ..
   71  brew install fftw3
   72  brew install fftw
   73  cmake ..
   74  CXX=g++-15 CC=gcc-15 make KISSFFT_DATATYPE=int16_t KISSFFT_STATIC=1 KISSFFT_OPENMP=1 all
   75  make install
   76  cd
   77  cd visual_sync_composer
   78  ls
   79  cd build
   80  cmake ..
   81  brew install rtaudio
   82  cmake ..
   83  cd
   84  git clone https://github.com/thestk/rtaudio
   85  cd rtaudio
   86  mkdir build
   87  cd build
   88  cmake ..
   89  make -j
   90  make install
   91  cd
   92  cd visual_sync_composer
   93  cd build
   94  make -j
   95  cmake ..
   96  brew install cpprestsdk
   97  cmake ..
   98  brew install nlohmannjson
   99  brew install nlohmann-json
  100  cmake ..
  101  brew install avahi
  102  brew install avahi-client
  103  nano ../CMakeLists.txt
  104  brew install avahi
  105  nano ../CMakeLists.txt
  106  brew install avahi
  107  cmake ..
  108  cd ..
  109  cd src
  110  git clone https://github.com/themultiplexer/f1_driver
  111  cd ..
  112  cd build
  113  cmake ..
  114  brew install hidapi
  115  cmake ..
  116  nano ../CMakeLists.txt
  117  cmake ..
  118  brew install boost
  119  cmake ..
  120  brew install boost-system
  121  nano ../CMakeLists.txt
  122  cmake ..
  123  nano ../CMakeLists.txt
  124  cmake ..
  125  nano ../CMakeLists.txt
  126  cmake ..
  127  brew install beast
  128  nano ../CMakeLists.txt
  129  cmake ..
  130  make -j
  131  nano ../src/f1_driver/include/controller_handler.h
  132  cmake ..
  133  make -j
  134  nano ../src/f1_driver/include/controller_handler.h
  135  make -j
  136  nano ../src/f1_driver/include/controller_handler.cpp
  137  nano ../src/f1_driver/srccontroller_handler.cpp
  138  nano ../src/f1_driver/src/controller_handler.cpp
  139  make -j
  140  nano ../CMakeLists.txt
  141  make -j
  142  nano ../src/main.cpp
  143  CC=gcc-15 make -j
  144  make clean
  145  CC=gcc-15 make -j
  146  ls
  147  CC=gcc-15 make -j
  148  nano ../CMakeLists.txt
  149  CC=gcc-15 make -j
  150  make -j
  151  nano ../CMakeLists.txt
  152  CC=gcc-15 make -j
  153  nano ../CMakeLists.txt
  154  make -j
  155  history
  156  history > install .sh
  157  history > install.sh
  158  ls
  159  cat install.sh
  160  history -h
  161  history -a
  162  cat 
  163  cat  ~/.zsh_sessions/55F2870B-9070-4705-8400-5B71E2824F72.historynew
  164  history 0
  165  history 0 > install.sh
  166  make -j
  167  git remote -v
  168  make -j -v
  169  nano ../CMakeLists.txt
  170  make -j -v
  171  make -j
  172  xcode
  173  nano ../CMakeLists.txt
  174  make -j
  175  cat ../src/main.cpp
  176  nano ../src/main.cpp
  177  make -j
  178  nano ../CMakeLists.txt
  179  cmake ..
  180  make -j
  181  nano ../CMakeLists.txt
  182  make -j
  183  make clean
  184  make VERBOSE=1 | tee -a out.txt
  185  cat out.txt| grep main.cpp
  186  cat out.txt| grep "main.cpp"
  187  cat out.txt| grep "main"
  188  cat out.txt
  189  cat out.txt| grep -i "main"
  190  make clean
  191  make -j
  192  nano ../CMakeLists.txt
  193  nano ../CMakeLists.txt
  194  git diff
  195  git commit -a
  196  git commit -a
  197  nano ../CMakeLists.txt
  198  cmake ..
  199  make -j
  200  brew install gl
  201  brew install opengl
  202  brew install glfw3
  203  make -j
  204  nano ../src/espnowsender.cpp
  205  make -j
  206  nano ../src/wifieventprocessor.cpp
  207  nano ../src/wifieventprocessor.h
  208  nano ../src/espnowsender.cpp
  209  nano ../src/espnowsender.h
  210  make -j
  211  nano ../src/mdnsflasher.h
  212  make -j
  213  nano ../src/mdnsflasher.cpp
  214  nano ../src/mdnsflasher.h
  215  git diff
  216  make -j
  217  nano ../CMakeLists.txt
  218  make -j
  219  brew install glew
  220  make -j
  221  nano ../src/midireceiver.h
  222  nano ../src/midireceiver.h
  223  make -j
  224  nano ../src/mdnsflasher.h
  225  nano ../src/mdnsflasher.cpp
  226  make -j
  227  nano ../src/midireceiver.cpp
  228  make -j
  229  nano ../src/espreceiver.cpp
  230  nano ../src/espreceiver.h
  231  make -j
  232  nano ../src/espreceiver.h
  233  make -j
  234  LDFLAGS="-v" make -j
  235  LDFLAGS="-framework OpenGL" make -j
  236  ls /usr/local/include
  237  brew --list glfw
  238  brew list glfw
  239  brew list glew
  240  nano ../src/espreceiver.cpp
  241  make -j
  242  nano ../src/espreceiver.h
  243  make -j
  244  nano ../src/espreceiver.h
  245  nano ../src/espnowsender.h
  246  nano ../src/espnowsender.cpp
  247  make -j
  248  nano ../src/netdevice.cpp
  249  nano ../src/netdevice.h
  250  make -j
  251  nano ../src/espreceiver.cpp
  252  make -j
  253  nano ../src/netdevice.cpp
  254  nano ../src/netdevice.cpp
  255  make -j
  256  nano ../src/netdevice.cpp
  257  make -j
  258  nano ../src/espreceiver.cpp
  259  git diff
  260  git checkout -- ../src/espnowsender.cpp
  261  git checkout -- ../src/espnowsender.h
  262  git checkout -- ../src/netdevice.h
  263  git checkout -- ../src/netdevice.cpp
  264  git checkout -- ../src/espreceiver.cpp
  265  nano ../src/netdevice.cpp
  266  nano ../src/netdevice.h
  267  nano ../src/espnowsender.h
  268  make -j
  269  nano ../src/espreceiver.cpp
  270  make -j
  271  nano ../src/espnowsender.cpp
  272  make -j
  273  nano ../src/wifieventprocessor.cpp
  274  make -j
  275  nano ../src/wifieventprocessor.h
  276  make -j
  277  nano ../src/espnowsender.h
  278  make -j
  279  nano ../src/wifieventprocessor.h
  280  make -j
  281  nano ../src/knobwidget.h
  282  make -j
  283  nano ../src/knobwidget.h
  284  nano ../src/oglwidget.h
  285  make -j
  286  nano ../src/tubewidget.h
  287  make -j
  288  nano ../CMakeLists.txt
  289  nano ../CMakeLists.txt
  290  make -j
  291  make clean
  292  cmake ..
  293  make
  294  nano ../CMakeLists.txt
  295  cmake ..
  296  rm -rf *
  297  cmake ..
  298  make -j
  299  nano ../src/radioselection.h
  300  make -j
  301  nano ../src/radioselection.h
  302  nano ../CMakeLists.txt
  303  make -j
  304  brew show qt6
  305  brew search qt6
  306  brew search qt
  307  nano ../CMakeLists.txt
  308  make -j
  309  cmake ..
  310  nano ../CMakeLists.txt
  311  make -j
  312  nano ../CMakeLists.txt
  313  nano ../CMakeLists.txt
  314  make -j
  315  nano ../CMakeLists.txt
  316  make -j
  317  git diff
  318  nano ../CMakeLists.txt
  319  make -j
  320  git diff
  321  nano ../CMakeLists.txt
  322  git diff
  323  make -j
  324  brew install glm
  325  make -j
  326  nano ../src/audiowindow.h
  327  make -j
  328  nano ../src/audiowindow.h
  329  make -j
  330  nano ../src/audiowindow.cpp
  331  make -j
  332  nano ../src/audiowindow.cpp
  333  make -j
  334  nano ../src/audiowindow.cpp
  335  make -j
  336  nano ../src/main
  337  nano ../src/main.cpp 
  338  make -j
  339  nano ../src/main.cpp 
  340  nano ../src/audiowindow.cpp
  341  make -j
  342  nano ../src/audiowindow.cpp
  343  make -j
  344  nano ../CMakeLists.txt
  345  make -j
  346  nano ../CMakeLists.txt
  347  make -j
  348  nano ../CMakeLists.txt
  349  make -j
  350  make -j
  351  make -j
  352  nano ../src/wifieventprocessor.cpp
  353  nano ../src/wifieventprocessor.cpp
  354  make -j
  355  nano ../src/wifieventprocessor.cpp
  356  make -j
  357  nano ../src/wifieventprocessor.cpp
  358  make -j
  359  nano ../CMakeLists.txt
  360  make -j
  361  nano ../CMakeLists.txt
  362  make -j
  363  ./VisualSyncComposer
  364  ./VisualSyncComposer
  365  ls
  366  open -n VisualSyncComposer.app
  367  lldb VisualSyncComposer_autogen
  368  lldb VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer 
  369  ls
  370  git checkout -- tubes.json
  371  git checkout -- effects.json
  372  git status .
  373  cp effects.json VisualSyncComposer.app/Contents/MacOS
  374  cp tubes.json VisualSyncComposer.app/Contents/MacOS
  375  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  376  cp effects.json VisualSyncComposer.app/Contents/
  377  cp tubes.json VisualSyncComposer.app/Contents/
  378  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  379  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  380  ls
  381  iconutil -c icns AppIcon.appiconset
  382  mv AppIcon.appiconset AppIcon.iconset
  383  iconutil -c icns AppIcon.iconset
  384  nano ../CMakeLists.txt
  385  make -j
  386  nano ../CMakeLists.txt
  387  make -j
  388  make -j
  389  nano ../CMakeLists.txt
  390  make -j
  391  nano ../CMakeLists.txt
  392  make -j
  393  make -j
  394  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  395  make -j
  396  make -j
  397  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  398  nano ../src/main.cpp
  399  make -j
  400  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  401  clear
  402  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  403  nano ../src/oglwidget.cpp
  404  make -j
  405  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  406  nano ../src/oglwidget.cpp
  407  make -j
  408  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  409  clear
  410  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  411  nano ../src/main.cpp
  412  nano ../src/audiowindow.cpp
  413  nano ../src/main.cpp
  414  make -j
  415  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  416  clear
  417  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  418  clear
  419  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  420  git diff
  421  cd ..
  422  git diff
  423  git commit -a --amend
  424  git checkout -b mac
  425  git remote remove origin
  426  git remote add origin git@github.com:themultiplexer/visual_sync_composer.git
  427  git push
  428  git push --set-upstream origin mac
  429  cat ~/.ssh/id
  430  ssh-keygen
  431  cat ~/.ssh/id_ed25519
  432  cat ~/.ssh/id_ed25519.pub
  433  git push
  434  git push --set-upstream origin mac
  435  git fetch --all
  436  git rebase
  437  make -j
  438  cd build
  439  cmake ..
  440  make -j
  441  nano ../src/espnowsender.cpp
  442  make -j
  443  ./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer
  444  git reset --hard
  445  git fetch --all
  446  git rebase
  447  ls
  448  git status
  449  cd 
  450  cd -
  451  cd ..
  452  git add show.icns
  453  git add src/f1_driver
  454  git rm --cached src/f1_driver
  455  git submodule add  src/f1_driver
  456  cd src/f1_driver
  457  git remote -v
  458  git submodule add https://github.com/themultiplexer/f1_driver src/f1_driver
  459  nano 
  460  cd ..
  461  cd ..
