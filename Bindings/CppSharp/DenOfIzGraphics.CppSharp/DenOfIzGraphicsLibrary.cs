using CppSharp.AST;
using CppSharp.Generators;
using CppSharp.Passes;

namespace CppSharp;

public class DenOfIzGraphicsLibrary(Config config) : ILibrary
{
    public void Setup(Driver driver)
    {
        var parserOptions = driver.ParserOptions;
        parserOptions.AddIncludeDirs(config.Includes);

        var options = driver.Options;
        options.GeneratorKind = GeneratorKind.CSharp;
        
        var module = options.AddModule("DenOfIzGraphics");
        module.IncludeDirs.Add(config.Includes);
        module.LibraryDirs.Add(config.InstallLocation);
        module.Libraries.Add(config.LibraryName);
        module.Headers.Add("DenOfIzGraphics.h");
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