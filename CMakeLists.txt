# Debug Release�Ɍ���
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE )

# �v���W�F�N�g��
project(Culling3D)

# CMake �o�[�W�����`�F�b�N
cmake_minimum_required(VERSION 2.8)

# MSVC�����ݒ�
if (MSVC)
	option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" OFF)
endif()

# �\�[�X�R�[�h�ݒ�
set(culling3d_src
	Culling3D/Culling3D.Grid.cpp
	Culling3D/Culling3D.Layer.cpp
	Culling3D/Culling3D.Matrix44.cpp
	Culling3D/Culling3D.ObjectInternal.cpp
	Culling3D/Culling3D.ReferenceObject.cpp
	Culling3D/Culling3D.Vector3DF.cpp
	Culling3D/Culling3D.WorldInternal.cpp)

# �R���p�C�����ǉ�
add_library( culling3d STATIC ${culling3d_src} )

# �t���O����
if (MSVC)
	if (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
		foreach (flag CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE)

			if (${flag} MATCHES "/MD")
				string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
			endif()
			if (${flag} MATCHES "/MDd")
				string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
			endif()
		endforeach()
	endif()
endif()


# �o�͐�̐ݒ�
if (MSVC)
set_target_properties(culling3d PROPERTIES OUTPUT_NAME_DEBUG "Culling3D" )
set_target_properties(culling3d PROPERTIES OUTPUT_NAME_RELEASE "Culling3D" )
else()
set_target_properties(culling3d PROPERTIES OUTPUT_NAME "Culling3D" )
endif()

install(
	TARGETS culling3d
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)