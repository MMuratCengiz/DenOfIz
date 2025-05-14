configure_file(${MAVEN_BASE_DIR}/pom.xml.in ${MAVEN_BASE_DIR}/pom.xml @ONLY CONTENT "@DENOFIZ_VERSION@=${DENOFIZ_VERSION}")

find_program(MAVEN_EXECUTABLE mvn HINTS $ENV{MAVEN_HOME}/bin)

if(NOT MAVEN_EXECUTABLE)
    message(WARNING "Maven not found. Maven package will not be built.")
else()
    # Build package
    add_custom_target(DenOfIzGraphicsJavaManaged
        COMMAND ${MAVEN_EXECUTABLE} clean package -f ${MAVEN_BASE_DIR}/pom.xml
        WORKING_DIRECTORY ${MAVEN_BASE_DIR}
        DEPENDS DenOfIzGraphicsJava
        COMMENT "Building Java Maven package"
    )

    # Install package locally
    add_custom_target(DenOfIzGraphicsJavaLocalInstall
        COMMAND ${MAVEN_EXECUTABLE} install -f ${MAVEN_BASE_DIR}/pom.xml
        WORKING_DIRECTORY ${MAVEN_BASE_DIR}
        DEPENDS DenOfIzGraphicsJavaManaged
        COMMENT "Installing Java Maven package to local repository"
    )

    # Publish to maven central
    add_custom_target(DenOfIzGraphicsJavaDeploy
        COMMAND ${MAVEN_EXECUTABLE} deploy -P release -f ${MAVEN_BASE_DIR}/pom.xml
        WORKING_DIRECTORY ${MAVEN_BASE_DIR}
        DEPENDS DenOfIzGraphicsJavaManaged
        COMMENT "Deploying Java Maven package to Maven Central"
    )
endif()