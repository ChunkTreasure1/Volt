using Microsoft.AspNetCore.Components;
using Microsoft.AspNetCore.Components.Web;

NexusWeb.WebApp app = new();
app.Build(args);
_ = app.Run();


namespace NexusWeb
{
    public class WebApp
    {
        private WebApplication? m_app = null;

        public void Build(string[] _args)
        {
            var builder = WebApplication.CreateBuilder(_args);
            builder.Services.AddRazorPages();
            builder.Services.AddServerSideBlazor();

            m_app = builder.Build();
        }

        public void Build()
        {
            WebApplicationOptions options = new()
            {
                ApplicationName = "",
                Args = new string[] { "" },
                ContentRootPath = "",
                EnvironmentName = "",
                WebRootPath = ""
            };

            var builder = WebApplication.CreateBuilder(options);
            builder.Services.AddRazorPages();
            builder.Services.AddServerSideBlazor();

            m_app = builder.Build();
        }

        public bool Run()
        {
            if (m_app is null) return false;

            if (!m_app.Environment.IsDevelopment())
            {
                // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
                m_app.UseHsts();
            }

            m_app.UseHttpsRedirection();
            m_app.UseStaticFiles();
            m_app.UseRouting();

            m_app.MapBlazorHub();
            m_app.MapFallbackToPage("/_Host");

            m_app.Run();
            return true;
        }
    }

}