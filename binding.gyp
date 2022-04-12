{
    'conditions': [
        ['OS=="win"', {
        'variables': {
            'MAGICK_ROOT%': '<!(python get_regvalue.py)',
            # download the dll binary and check off for libraries and includes
            'OSX_VER%': "0",
        }
        }],
        ['OS=="mac"', {
        'variables': {
            # matches 10.9.X , 10.10 and outputs 10.9, 10.10, 10.11, 10.12, 10.13
            'OSX_VER%': "<!(sw_vers | grep 'ProductVersion:' | grep -o '10.[0-9]*')",
        }
        }, {
        'variables': {
            'OSX_VER%': "0",
        }
        }]
    ],
    "targets": [
        {
            "target_name": "qpdf",
            "sources": [ "src/qpdf.cc" ],
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "/media/mishatre/Fast1/projects/node-qpdf-native/qpdf/include",
                "/media/mishatre/Fast1/projects/node-qpdf-native/qpdf/build/libqpdf"
            ],
            "conditions": [
                ['OS=="linux" or OS=="solaris" or OS=="freebsd"', { # not windows not mac
                    "libraries": [
                        '<!@(pkg-config --libs libqpdf)',
                        "-ljpeg",
                        "-lz",
                    ],
                    'cflags': [
                        '<!@(pkg-config --cflags libqpdf)'
                    ],
                    "library_dirs": [
                        "/media/mishatre/Fast1/projects/node-qpdf-native/qpdf/build/libqpdf"
                    ]
                }]
            ]
        }
    ]
}