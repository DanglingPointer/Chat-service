using System;
using System.ServiceModel;
using ChatServiceLib;

namespace ChatServiceHost
{
    class Program
    {
        static void Main(string[] args)
        {
            using (ServiceHost host = new ServiceHost(typeof(SimpleChatServer))) {
                host.Open();
                Console.WriteLine("Service open, press to close\n");

                foreach (var se in host.Description.Endpoints) {
                    Console.WriteLine("Adress: {0}", se.Address);
                    Console.WriteLine("Binding: {0}", se.Binding.Name);
                    Console.WriteLine("Contract: {0}\n", se.Contract.Name);
                }
                Console.ReadKey();
            }
        }
    }
}
