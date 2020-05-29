using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;

public class VisLinkClient : MonoBehaviour
{
    [DllImport("VLUnityConnector")]
    private static extern int createClientAPI(string key, int port);
    [DllImport("VLUnityConnector")]
    private static extern IntPtr GetCreateAPIFunc();
    [DllImport("VLUnityConnector")]
    private static extern bool isAPIReady(int api);

    public string address = "127.0.0.1";
    public int port = 3457;
    private int api;
    private bool init = false;

    public int GetAPI()
    {
        return api;
    }

    public bool IsReady()
    {
        return isAPIReady(api);
    }

    // Start is called before the first frame update
    void Start()
    {
        api = createClientAPI(address, port);
        GL.IssuePluginEvent(GetCreateAPIFunc(), api);
        Debug.Log("Client API created");
    }


    // Update is called once per frame
    void Update()
    {
        if (!init)
        {
            init = IsReady();
        }
        
    }
}
