using CppSharp.AST;

namespace CppSharp;

public class DenOfIzGraphicsLibrary(Config config) : ILibrary
{
    public void Setup(Driver driver)
    {
        var options = driver.Options;
        var module = options.AddModule("DenOfIzGraphics");
        module.Headers.Add("DenOfIzGraphics.h");

        var parserOptions = driver.ParserOptions;
        parserOptions.AddIncludeDirs(config.Includes);
    }

    public void Preprocess(Driver driver, ASTContext ctx)
    {
        throw new NotImplementedException();
    }

    public void Postprocess(Driver driver, ASTContext ctx)
    {
        throw new NotImplementedException();
    }

    public void SetupPasses(Driver driver)
    {
        throw new NotImplementedException();
    }
}