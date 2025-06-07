using System;
using System.IO;
using CppSharp.AST;
using CppSharp.Generators;
using CppSharp.Passes;
using CppSharp.Parser;

namespace CppSharp;

public class DenOfIzGraphicsLibrary(Config config) : ILibrary
{
    public void Setup(Driver driver)
    {
        var parserOptions = driver.ParserOptions;
        parserOptions.AddIncludeDirs(config.Includes);
        parserOptions.LanguageVersion = LanguageVersion.CPP20;
        parserOptions.EnableRTTI = true;
        if (OperatingSystem.IsWindows())
        {
            parserOptions.SetupMSVC(VisualStudioVersion.VS2022);
            parserOptions.AddDefines("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH");
        }
        
        var options = driver.Options;
        options.GeneratorKind = GeneratorKind.CSharp;
        options.OutputDir = config.OutputDir;

        var module = options.AddModule("DenOfIzGraphics");
        module.IncludeDirs.Add(config.Includes);
        module.LibraryDirs.Add(config.LibraryDir);
        module.Libraries.Add(config.LibraryName);
        module.Headers.Add("DenOfIzGraphics/DenOfIzGraphics.h");
        
        module.OutputNamespace = "DenOfIz";
    }

    public void Preprocess(Driver driver, ASTContext ctx)
    {
    }

    public void Postprocess(Driver driver, ASTContext ctx)
    {
    }

    public void SetupPasses(Driver driver)
    {
        driver.Context.TranslationUnitPasses.RenameDeclsUpperCase(RenameTargets.Any);
        driver.Context.TranslationUnitPasses.AddPass(new FunctionToInstanceMethodPass());
        driver.Context.TranslationUnitPasses.AddPass(new FixDefaultParamValuesOfOverridesPass());
    }
}