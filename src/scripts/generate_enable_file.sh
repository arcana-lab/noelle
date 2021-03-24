#!/bin/bash

enableFile="../enable" ;

echo "#!/bin/bash" > ${enableFile} ;
echo "" >> ${enableFile} ;

repoPath="`pwd`/../" ;
noelleDir="`realpath ${repoPath}`" ;
echo "NOELLE_HOME=${noelleDir}" >> ${enableFile} ;
echo "export PATH=\${NOELLE_HOME}/install/bin:\$PATH" >> ${enableFile} ;
echo "export LD_LIBRARY_PATH=\${NOELLE_HOME}/install/lib:\$LD_LIBRARY_PATH" >> ${enableFile} ;
