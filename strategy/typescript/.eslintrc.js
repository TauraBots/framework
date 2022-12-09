/*
👋 Hi! This file was autogenerated by tslint-to-eslint-config.
https://github.com/typescript-eslint/tslint-to-eslint-config

It represents the closest reasonable ESLint configuration to this
project's original TSLint configuration.

We recommend eventually switching this configuration to extend from
the recommended rulesets in typescript-eslint. 
https://github.com/typescript-eslint/tslint-to-eslint-config/blob/master/docs/FAQs.md

Happy linting! 💖
*/
module.exports = {
    "env": {
        "es6": true
    },
    "parser": "@typescript-eslint/parser",
    "parserOptions": {
        "project": "tsconfig.json",
        "sourceType": "module"
    },
    "plugins": [
        "eslint-plugin-no-null",
        "eslint-plugin-jsdoc",
        "eslint-plugin-import",
        "eslint-plugin-unicorn",
        "@typescript-eslint",
        "@typescript-eslint/tslint"
    ],
    "root": true,
    "rules": {
        "@typescript-eslint/adjacent-overload-signatures": "error",
        "@typescript-eslint/ban-types": [
            "error",
            {
                "types": {
                    "Boolean": {
                        "message": "Avoid using the 'Boolean' type. Did you mean 'boolean'?"
                    },
                    "Number": {
                        "message": "Avoid using the 'Number' type. Did you mean 'number'?"
                    },
                    "String": {
                        "message": "Avoid using the 'String' type. Did you mean 'string'?"
                    },
                    "Symbol": {
                        "message": "Avoid using the 'Symbol' type. Did you mean 'symbol'?"
                    },
                    "Function": false,
                    "Object": false
                }
            }
        ],
        "@typescript-eslint/indent": [
            // TODO
            "off",
            "tab",
            {
                // TODO
                "ArrayExpression": "off",
                "CallExpression": {
                    // TODO sometimes 1 and sometimes 2
                    "arguments": "off"
                },
                "FunctionDeclaration": {
                    "parameters": 2
                },
                "FunctionExpression": {
                    // TODO sometimes 1 and sometimes 2
                    "parameters": "off"
                },
                "SwitchCase": 1,
                "ignoredNodes": [
                    "ConditionalExpression",
                    "TSUnionType",
                    "TSFunctionType",
                    "TSDeclareFunction",
                ]
            }
        ],
        "@typescript-eslint/member-delimiter-style": [
            "error",
            {
                "multiline": {
                    "delimiter": "semi",
                    "requireLast": true
                },
                "singleline": {
                    "delimiter": "semi",
                    "requireLast": false
                }
            }
        ],
        // "@typescript-eslint/naming-convention": "error",
        "@typescript-eslint/no-misused-new": "error",
        "@typescript-eslint/no-parameter-properties": "error",
        "@typescript-eslint/no-require-imports": "error",
        "@typescript-eslint/no-this-alias": "error",
        "@typescript-eslint/prefer-function-type": "error",
        "@typescript-eslint/quotes": [
            "error",
            "double",
            {
                "avoidEscape": true
            }
        ],
        "@typescript-eslint/semi": [
            "error",
            "always"
        ],
        "arrow-body-style": "off",
        "arrow-parens": [
            "error",
            "always"
        ],
        "brace-style": [
            "error",
            "1tbs",
            {
                "allowSingleLine": true
            }
        ],
        "constructor-super": "error",
        "curly": [
            "error",
            "multi-line"
        ],
        "eol-last": "error",
        "import/order": "error",
        "jsdoc/check-alignment": "error",
        "jsdoc/check-indentation": "off",
        // "jsdoc/newline-after-description": "error",
        "linebreak-style": [
            "error",
            "unix"
        ],
        "new-parens": "error",
        "no-bitwise": "warn",
        "no-console": [
            "error",
            {
                "allow": [
                    "warn",
                    "dir",
                    "time",
                    "timeEnd",
                    "timeLog",
                    "trace",
                    "assert",
                    "clear",
                    "count",
                    "countReset",
                    "group",
                    "groupEnd",
                    "table",
                    "debug",
                    "info",
                    "dirxml",
                    "groupCollapsed",
                    "Console",
                    "profile",
                    "profileEnd",
                    "timeStamp",
                    "context",
                    "createTask"
                ]
            }
        ],
        "no-debugger": "error",
        "no-duplicate-case": "error",
        "no-duplicate-imports": "error",
        "no-empty": [
            "error",
            {
                "allowEmptyCatch": true
            }
        ],
        "no-eval": "error",
        "no-invalid-this": "error",
        "no-new-wrappers": "error",
        "no-null/no-null": "error",
        "no-redeclare": "off",
        "@typescript-eslint/no-redeclare": [
            "error",
            {
                "ignoreDeclarationMerge": true
            }
        ],
        "no-return-await": "error",
        "eqeqeq": "off",
        // this is a workaround, because eqeqeq does not allow making undefined an exception
        "no-restricted-syntax": [
            "error",
            {
                "selector": "BinaryExpression[operator='=='][left.name!='undefined'][right.name!='undefined']",
                "message": "Comparison with == is only allowed when comparing with undefined, because JavaScript is cursed. Use === instead."
            },
            {
                "selector": "BinaryExpression[operator='!='][left.name!='undefined'][right.name!='undefined']",
                "message": "Comparison with != is only allowed when comparing with undefined, because JavaScript is cursed. Use !== instead."
            },
        ],
        "no-sparse-arrays": "error",
        "no-template-curly-in-string": "error",
        "no-throw-literal": "error",
        "no-trailing-spaces": "error",
        "no-unused-labels": "error",
        "no-var": "error",
        "prefer-object-spread": "error",
        "prefer-template": "error",
        "quote-props": [
            "error",
            "consistent"
        ],
        "radix": "error",

        // turned off because there are overriding @typescript versions
        "quotes": "off",
        "semi": "off",
        "indent": "off",

        "space-before-function-paren": [
            "error",
            "never"
        ],
        "space-in-parens": [
            "error",
            "never"
        ],
        "spaced-comment": [
            "error",
            "always",
            {
                "line": {
                    "markers": [
                        "/"
                    ]
                },
                "block": {
                    "balanced": true,
                    "exceptions": ["*"],
                    "markers": [
                        "*"
                    ]
                }
            }
        ],
        "unicorn/prefer-switch": [
            "error",
            {
                "minimumCases": 5
            }
        ],
        "use-isnan": "error",
        "@typescript-eslint/tslint/config": [
            "error",
            {
                "rules": {
                    "encoding": true,
                    "import-spacing": true,
                    "no-unnecessary-callback-wrapper": true,
                    "prefer-while": true,
                    "whitespace": [
                        true,
                        "check-branch",
                        "check-decl",
                        "check-operator",
                        "check-module",
                        "check-seperator",
                        "check-rest-spread",
                        "check-type",
                        "check-typecast",
                        "check-type-operator",
                        "check-preblock"
                    ]
                }
            }
        ]
    }
};
