#!/bin/bash

# Set the installation directory
installDir=$NOELLE_INSTALL_DIR ;
if test "$installDir" == "" ; then
  installDir="`git rev-parse --show-toplevel`/install"  ;
fi
installDir="`realpath ${installDir}/`" ;
mkdir -p $installDir ;

# Set the enable file
enableFile="../enable" ;

echo "#!/bin/bash" > ${enableFile} ;
echo "" >> ${enableFile} ;

echo "NOELLE_HOME=${installDir}" >> ${enableFile} ;
echo "export PATH=\${NOELLE_HOME}/bin:\$PATH" >> ${enableFile} ;
echo "export LD_LIBRARY_PATH=\${NOELLE_HOME}/lib:\$LD_LIBRARY_PATH" >> ${enableFile} ;
