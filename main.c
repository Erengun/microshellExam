#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int tmp;

int cout(char *str)
{
    while (*str)
        write(2, str++, 1);
    return 1;
}

int change_directory(char **str, int index)
{
    if (index != 2)
        return cout("error: cd: bad arguments\n");
    if (chdir(str[1]))
        return cout("error: cd: cannot change directory to ") & cout(str[1]) & cout("\n");
    return 0;
}

int setup_pipe(int pip, int *tmp2)
{
    if (pip && (pipe(tmp2)))
        return cout("error: fatal\n");
    return 0;
}

int child_process_execution(char **str, char **env, int index, int tmp, int *tmp2, int pip)
{
    str[index] = 0;
    if (dup2(tmp, 0) == -1 || close(tmp) == -1 || (pip && (dup2(tmp2[1], 1) == -1 || close(tmp2[0]) == -1 || close(tmp2[1]) == -1)))
        return cout("error: fatal\n");
    execve(*str, str, env);
    return cout("error: cannot execute ") & cout(*str) & cout("\n");
}

int parent_process_execution(int pid, int tmp, int *tmp2, int pip, int *res)
{
    if ((pip && (dup2(tmp2[0], tmp) == -1 || close(tmp2[0]) == -1 || close(tmp2[1]) == -1)) || (!pip && dup2(0, tmp) == -1) || waitpid(pid, res, 0) == -1)
        return cout("error: fatal\n");
    return 0;
}

int execute(char **str, char **env, int index)
{
    int tmp2[2], res;
    int pip = (str[index] && !strcmp(str[index], "|"));

    if (setup_pipe(pip, tmp2))
        return 1;

    int pid = fork();
    if (!pid)
        return child_process_execution(str, env, index, tmp, tmp2, pip);

    if (parent_process_execution(pid, tmp, tmp2, pip, &res))
        return 1;

    return WIFEXITED(res) && WEXITSTATUS(res);
}

int main(int ac, char **av, char **env)
{
    (void)ac;
    int index = 0;
    int res = 0;
    tmp = dup(0);

    while (av[index] && av[++index])
    {
        av = av + index;
        index = 0;
        while (av[index] && strcmp(av[index], "|") && strcmp(av[index], ";"))
            index++;
        if (!strcmp(*av, "cd"))
            res = change_directory(av, index);
        else if (index)
            res = execute(av, env, index);
    }

    return (dup2(0, tmp) == -1 && cout("error: fatal\n")) | res;
}
